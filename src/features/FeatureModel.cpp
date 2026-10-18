#include "features/FeatureModel.h"

#include "core/geometry/GeometricTolerance.h"
#include "features/AxialCut.h"
#include "features/AxialSubtraction.h"
#include "features/GeneralExtrusion.h"
#include "features/PlanarRegion.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace microsw::features
{
const char* semanticFaceRoleName(ExtrudeCapRole role) noexcept
{
    switch (role)
    {
    case ExtrudeCapRole::StartCap: return "Start cap";
    case ExtrudeCapRole::EndCap: return "End cap";
    case ExtrudeCapRole::InheritedStartCap: return "Inherited start cap";
    case ExtrudeCapRole::InheritedEndCap: return "Inherited end cap";
    case ExtrudeCapRole::PocketFloor: return "Pocket floor";
    }
    return "Generated face";
}

const sketch::SketchPlane& resolveSemanticFacePlane(
    const SolidFeatureCache& cache, ExtrudeCapRole role)
{
    const SemanticFaceProvenance* match{};
    std::size_t count{};
    for (const auto& provenance : cache.semanticFaces)
        if (provenance.role == role) { match = &provenance; ++count; }
    if (count == 0)
        throw std::runtime_error{"The sketch support face is unavailable after rebuild."};
    if (count != 1)
        throw std::runtime_error{"The sketch support face is ambiguous after rebuild."};
    return match->sketchPlane;
}
namespace
{
constraints::SolverPolicy featureSolverPolicy()
{
    auto policy = constraints::SolverPolicy{};
    // Feature rebuild feeds solved geometry into the modeling kernel. Tighten
    // this integration policy so coincident endpoints also satisfy the kernel's
    // geometric connectivity tolerance after a driving-dimension edit.
    policy.residualSatisfactionTolerance = 1.0e-10;
    policy.initialStateRegularization = 1.0e-12;
    return policy;
}

bool samePlane(const geometry::Plane& a, const geometry::Plane& b)
{
    return std::abs(std::abs(math::dot(a.normal(), b.normal())) - 1.0)
            <= geometry::defaultGeometricTolerance
        && a.contains(b.origin());
}

void addSemanticFaces(SolidFeatureCache& cache, ExtrudeCapRole role,
                      const geometry::Plane& plane, const sketch::SketchPlane& sketchPlane)
{
    for (std::size_t body = 0; body < cache.solids.size(); ++body)
        for (std::size_t face = 0; face < cache.solids[body].faces().size(); ++face)
        {
            const auto& candidate = cache.solids[body].faces()[face];
            if (const auto* support = std::get_if<geometry::Plane>(&candidate.support());
                support && samePlane(*support, plane))
                cache.semanticFaces.push_back({role, body,
                    topology::FaceId{static_cast<topology::FaceId::Value>(face)}, sketchPlane});
        }
}
}

std::vector<FeatureId> FeatureRecord::dependencies() const
{
    return std::visit([](const auto& value) -> std::vector<FeatureId> {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, SketchFeature>) {
            if (const auto* support = std::get_if<SemanticFaceSupport>(&value.support))
                return {support->sourceFeatureId};
            return {};
        }
        else if constexpr (std::is_same_v<T, ExtrudeFeature>) return {value.sourceSketchFeatureId};
        else if constexpr (std::is_same_v<T, CutExtrudeFeature>) return {value.sourceSketchFeatureId, value.targetResultFeatureId};
        else return {value.inputA, value.inputB};
    }, data_);
}

FeatureModel::FeatureModel() : solver_{featureSolverPolicy()} {}
FeatureModel::FeatureModel(constraints::ConstraintSolver solver) : solver_{std::move(solver)} {}

std::string FeatureModel::nextName(FeatureKind kind) const
{
    const char* prefix = kind == FeatureKind::Sketch ? "Sketch" : kind == FeatureKind::Extrude ? "Extrude" : kind == FeatureKind::CutExtrude ? "CutExtrude" : "Boolean";
    std::size_t largest{};
    const std::string prefixText{prefix};
    for (const auto& record : records_)
        if (record.kind() == kind && record.displayName().starts_with(prefixText))
            try { largest = std::max(largest, static_cast<std::size_t>(
                std::stoul(record.displayName().substr(prefixText.size())))); }
            catch (const std::exception&) {}
    return prefixText + std::to_string(largest + 1);
}

FeatureId FeatureModel::addSketch(sketch::Sketch sketch)
{
    const auto id = FeatureId{nextFeatureId_};
    if (!id.isValid()) throw std::overflow_error{"Feature ID space exhausted"};
    records_.push_back(FeatureRecord{id, FeatureKind::Sketch, nextName(FeatureKind::Sketch), SketchFeature{std::move(sketch)}});
    ++nextFeatureId_;
    return id;
}

FeatureId FeatureModel::addFaceSketch(sketch::Sketch sketch, FeatureId sourceExtrude,
                                      ExtrudeCapRole role)
{
    const auto sourceKind = find(sourceExtrude).kind();
    const auto extrudeRole = role == ExtrudeCapRole::StartCap || role == ExtrudeCapRole::EndCap;
    const auto cutRole = role == ExtrudeCapRole::InheritedStartCap
        || role == ExtrudeCapRole::InheritedEndCap || role == ExtrudeCapRole::PocketFloor;
    if ((extrudeRole && sourceKind != FeatureKind::Extrude)
        || (cutRole && sourceKind != FeatureKind::CutExtrude))
        throw std::invalid_argument{"Semantic face role is not supported by the source feature"};
    const auto id = FeatureId{nextFeatureId_};
    if (!id.isValid()) throw std::overflow_error{"Feature ID space exhausted"};
    records_.push_back(FeatureRecord{id, FeatureKind::Sketch, nextName(FeatureKind::Sketch),
        SketchFeature{std::move(sketch), SemanticFaceSupport{sourceExtrude, role}}});
    ++nextFeatureId_;
    return id;
}

FeatureId FeatureModel::addExtrude(FeatureId sketchId, math::Scalar distance)
{
    if (find(sketchId).kind() != FeatureKind::Sketch)
        throw std::invalid_argument{"Extrude source must be an earlier SketchFeature"};
    if (!std::isfinite(distance) || distance <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Extrusion distance must exceed geometric tolerance"};
    const auto id = FeatureId{nextFeatureId_};
    if (!id.isValid()) throw std::overflow_error{"Feature ID space exhausted"};
    records_.push_back(FeatureRecord{id, FeatureKind::Extrude, nextName(FeatureKind::Extrude), ExtrudeFeature{sketchId, distance}});
    ++nextFeatureId_;
    resultFeatureId_ = id;
    return id;
}

FeatureId FeatureModel::addCutExtrude(FeatureId sketchId, FeatureId targetResultId,
                                      math::Scalar distance, CutDirection direction)
{
    if (find(sketchId).kind() != FeatureKind::Sketch)
        throw std::invalid_argument{"Cut Extrude source must be an earlier Sketch"};
    if (!isSolidProducer(targetResultId))
        throw std::invalid_argument{"Cut Extrude target must be an earlier modeled result"};
    if (targetResultId.value() >= sketchId.value())
        throw std::invalid_argument{"Cut Extrude Sketch must follow its target result"};
    if (!std::isfinite(distance) || distance <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Cut Extrude distance must exceed geometric tolerance"};
    const auto id = FeatureId{nextFeatureId_};
    if (!id.isValid()) throw std::overflow_error{"Feature ID space exhausted"};
    records_.push_back(FeatureRecord{id, FeatureKind::CutExtrude,
        nextName(FeatureKind::CutExtrude), CutExtrudeFeature{sketchId, targetResultId, distance, direction}});
    ++nextFeatureId_;
    resultFeatureId_ = id;
    return id;
}

FeatureId FeatureModel::addBoolean(FeatureId inputA, FeatureId inputB, modeling::BooleanOperation operation)
{
    if (!isSolidProducer(inputA) || !isSolidProducer(inputB))
        throw std::invalid_argument{"Boolean inputs must be earlier Solid-producing features"};
    const auto id = FeatureId{nextFeatureId_};
    if (!id.isValid()) throw std::overflow_error{"Feature ID space exhausted"};
    records_.push_back(FeatureRecord{id, FeatureKind::Boolean, nextName(FeatureKind::Boolean), BooleanFeature{inputA, inputB, operation}});
    ++nextFeatureId_;
    resultFeatureId_ = id;
    return id;
}

const FeatureRecord& FeatureModel::find(FeatureId id) const
{
    if (const auto* record = tryFind(id)) return *record;
    throw std::out_of_range{"Invalid FeatureId"};
}
const FeatureRecord* FeatureModel::tryFind(FeatureId id) const noexcept
{
    if (!id.isValid()) return nullptr;
    const auto found = std::lower_bound(records_.begin(), records_.end(), id,
        [](const FeatureRecord& record, FeatureId value) { return record.id() < value; });
    return found != records_.end() && found->id() == id ? &*found : nullptr;
}
FeatureRecord& FeatureModel::mutableFind(FeatureId id)
{
    return const_cast<FeatureRecord&>(std::as_const(*this).find(id));
}
std::optional<ExtrudeCapRole> FeatureModel::semanticFaceRole(
    FeatureId source, std::size_t bodyIndex, topology::FaceId faceId) const
{
    const auto& record = find(source);
    if (!record.cache()) return std::nullopt;
    const auto* cache = std::get_if<SolidFeatureCache>(&*record.cache());
    if (!cache || cache->solids.size() != 1) return std::nullopt;
    std::optional<ExtrudeCapRole> result;
    for (const auto& provenance : cache->semanticFaces)
        if (provenance.bodyIndex == bodyIndex && provenance.faceId == faceId)
        {
            if (result && *result != provenance.role) return std::nullopt;
            result = provenance.role;
        }
    if (result && std::count_if(cache->semanticFaces.begin(), cache->semanticFaces.end(),
            [&](const SemanticFaceProvenance& provenance) { return provenance.role == *result; }) != 1)
        return std::nullopt;
    return result;
}
bool FeatureModel::isSolidProducer(FeatureId id) const
{
    const auto kind = find(id).kind();
    return kind == FeatureKind::Extrude || kind == FeatureKind::Boolean || kind == FeatureKind::CutExtrude;
}

void FeatureModel::markDirtyClosure(FeatureId id)
{
    mutableFind(id).state_ = FeatureState::Dirty;
    mutableFind(id).diagnostic_.clear();
    for (auto& record : records_)
        for (const auto dependency : record.dependencies())
            if (dependency == id && record.state_ != FeatureState::Dirty) markDirtyClosure(record.id_);
}

void FeatureModel::editSketch(FeatureId id, const SketchEdit& edit)
{
    auto& record = mutableFind(id);
    if (record.kind() != FeatureKind::Sketch) throw std::invalid_argument{"Feature is not a SketchFeature"};
    auto candidate = std::get<SketchFeature>(record.data_).sketch;
    edit(candidate);
    std::get<SketchFeature>(record.data_).sketch = std::move(candidate);
    markDirtyClosure(id);
}
void FeatureModel::setExtrudeDistance(FeatureId id, math::Scalar distance)
{
    if (!std::isfinite(distance) || distance <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Extrusion distance must exceed geometric tolerance"};
    auto& record = mutableFind(id);
    if (record.kind() != FeatureKind::Extrude) throw std::invalid_argument{"Feature is not an ExtrudeFeature"};
    std::get<ExtrudeFeature>(record.data_).distance = distance;
    markDirtyClosure(id);
}
void FeatureModel::setCutExtrude(FeatureId id, math::Scalar distance, CutDirection direction)
{
    if (!std::isfinite(distance) || distance <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Cut Extrude distance must exceed geometric tolerance"};
    auto& record = mutableFind(id);
    if (record.kind() != FeatureKind::CutExtrude)
        throw std::invalid_argument{"Feature is not a CutExtrudeFeature"};
    auto& cut = std::get<CutExtrudeFeature>(record.data_);
    cut.distance = distance; cut.direction = direction;
    markDirtyClosure(id);
}
void FeatureModel::setBooleanOperation(FeatureId id, modeling::BooleanOperation operation)
{
    auto& record = mutableFind(id);
    if (record.kind() != FeatureKind::Boolean) throw std::invalid_argument{"Feature is not a BooleanFeature"};
    std::get<BooleanFeature>(record.data_).operation = operation;
    markDirtyClosure(id);
}
void FeatureModel::setResultFeatureId(FeatureId id)
{
    if (!isSolidProducer(id)) throw std::invalid_argument{"Result feature must produce a Solid or EMPTY"};
    resultFeatureId_ = id;
}

std::vector<FeatureId> FeatureModel::deletionClosure(FeatureId id) const
{
    (void)find(id);
    std::vector<FeatureId> closure{id};
    for (const auto& record : records_)
    {
        const auto dependencies = record.dependencies();
        if (std::any_of(dependencies.begin(), dependencies.end(),
                [&](FeatureId dependency) {
                    return std::find(closure.begin(), closure.end(), dependency) != closure.end();
                }))
            closure.push_back(record.id());
    }
    return closure;
}

std::vector<FeatureId> FeatureModel::deleteFeature(FeatureId id)
{
    const auto closure = deletionClosure(id);
    records_.erase(std::remove_if(records_.begin(), records_.end(), [&](const FeatureRecord& record) {
        return std::find(closure.begin(), closure.end(), record.id()) != closure.end();
    }), records_.end());
    resultFeatureId_ = {};
    for (auto record = records_.rbegin(); record != records_.rend(); ++record)
        if (isSolidProducer(record->id()) && record->state() == FeatureState::Clean
            && record->cache())
        {
            resultFeatureId_ = record->id();
            break;
        }
    return closure;
}

RebuildResult FeatureModel::rebuild()
{
    std::vector<std::optional<FeatureCache>> candidates(nextFeatureId_);
    std::vector<bool> affected(nextFeatureId_);
    for (const auto& record : records_) affected[record.id().value()] = record.state() != FeatureState::Clean;
    std::optional<FeatureId> failure;
    std::string diagnostic;
    std::size_t blocked{};
    const auto output = [&](FeatureId id) -> const FeatureCache& {
        if (candidates[id.value()]) return *candidates[id.value()];
        const auto& cache = find(id).cache();
        if (!cache) throw std::runtime_error{"Dependency has no successful output"};
        return *cache;
    };
    std::function<std::optional<math::Vector3>(FeatureId)> targetAxis;
    targetAxis = [&](FeatureId id) -> std::optional<math::Vector3> {
        const auto& feature = find(id);
        if (feature.kind() == FeatureKind::Extrude)
        {
            const auto& extrusion = std::get<ExtrudeFeature>(feature.data());
            return std::get<SketchFeatureCache>(
                output(extrusion.sourceSketchFeatureId)).solvedSketch.plane().normal();
        }
        if (feature.kind() == FeatureKind::CutExtrude)
            return targetAxis(std::get<CutExtrudeFeature>(feature.data()).targetResultFeatureId);
        if (feature.kind() == FeatureKind::Boolean)
        {
            const auto& boolean = std::get<BooleanFeature>(feature.data());
            const auto first = targetAxis(boolean.inputA);
            const auto second = targetAxis(boolean.inputB);
            if (!first || !second
                || classifyAxialCut(*first, *second) == AxialCutAlignment::Unsupported)
                return std::nullopt;
            return first;
        }
        return std::nullopt;
    };
    for (auto& record : records_)
    {
        if (!affected[record.id().value()]) continue;
        bool upstreamFailed = false;
        for (const auto dependency : record.dependencies())
            if (affected[dependency.value()] && !candidates[dependency.value()]) upstreamFailed = true;
        if (upstreamFailed)
        {
            record.state_ = FeatureState::BlockedByDependency;
            record.diagnostic_ = "Upstream feature failed";
            ++blocked;
            continue;
        }
        try
        {
            if (record.kind() == FeatureKind::Sketch)
            {
                const auto& data = std::get<SketchFeature>(record.data_);
                auto solved = data.sketch;
                if (const auto* support = std::get_if<SemanticFaceSupport>(&data.support))
                {
                    const auto& source = find(support->sourceFeatureId);
                    const auto& sourceCache = std::get<SolidFeatureCache>(output(source.id()));
                    if (sourceCache.solids.size() != 1)
                        throw std::runtime_error{"Sketch support requires a single-body generated face"};
                    solved = solved.withPlane(resolveSemanticFacePlane(sourceCache, support->capRole));
                }
                if (solved.constraintCount() != 0)
                {
                    const auto result = solver_.solve(solved);
                    if (result.status != constraints::SolveStatus::Solved)
                        throw std::runtime_error{"Sketch constraint solve failed"};
                }
                candidates[record.id().value()] = SketchFeatureCache{std::move(solved)};
            }
            else if (record.kind() == FeatureKind::Extrude)
            {
                const auto& data = std::get<ExtrudeFeature>(record.data_);
                const auto& sketchCache = std::get<SketchFeatureCache>(output(data.sourceSketchFeatureId));
                const auto regions = extractPlanarRegions(sketchCache.solvedSketch);
                auto extrusion = extrudeRegions(regions, sketchCache.solvedSketch.plane(), data.distance);
                if (!extrusion.succeeded())
                    throw std::runtime_error{extrusion.diagnostic};
                SolidFeatureCache cache{std::move(extrusion.solids)};
                const auto& p = sketchCache.solvedSketch.plane();
                addSemanticFaces(cache, ExtrudeCapRole::StartCap,
                    geometry::Plane{p.origin(), p.normal()},
                    sketch::SketchPlane{p.origin(), p.xAxis(), p.yAxis() * -1.0});
                const auto endOrigin = p.origin() + p.normal() * data.distance;
                addSemanticFaces(cache, ExtrudeCapRole::EndCap,
                    geometry::Plane{endOrigin, p.normal()},
                    sketch::SketchPlane{endOrigin, p.xAxis(), p.yAxis()});
                candidates[record.id().value()] = std::move(cache);
            }
            else if (record.kind() == FeatureKind::Boolean)
            {
                const auto& data = std::get<BooleanFeature>(record.data_);
                const auto& aCache = std::get<SolidFeatureCache>(output(data.inputA));
                const auto& bCache = std::get<SolidFeatureCache>(output(data.inputB));
                const auto supportsRestrictedBoolean = [](const SolidFeatureCache& cache) {
                    return cache.solids.size() == 1
                        && std::all_of(cache.solids.front().edges().begin(),
                            cache.solids.front().edges().end(), [](const auto& edge) {
                                return std::holds_alternative<geometry::Segment3>(edge.geometry());
                            });
                };
                if (!supportsRestrictedBoolean(aCache) || !supportsRestrictedBoolean(bCache))
                    throw std::runtime_error{
                        "Boolean supports one axis-aligned linear solid per input"};
                const auto& a = aCache.solids.front();
                const auto& b = bCache.solids.front();
                auto result = modeling::booleanOperation(a, b, data.operation);
                if (result.status() == modeling::BooleanStatus::SUCCESS)
                    candidates[record.id().value()] = SolidFeatureCache{{*result.solid()}, {}};
                else if (result.status() == modeling::BooleanStatus::EMPTY)
                    candidates[record.id().value()] = EmptyFeatureCache{};
                else throw std::runtime_error{"Boolean rebuild failed"};
            }
            else
            {
                const auto& data = std::get<CutExtrudeFeature>(record.data_);
                const auto& sketchCache = std::get<SketchFeatureCache>(
                    output(data.sourceSketchFeatureId));
                auto cutDirection = sketchCache.solvedSketch.plane().normal();
                if (data.direction == CutDirection::Reverse)
                    cutDirection = -cutDirection;
                const auto axis = targetAxis(data.targetResultFeatureId);
                if (!axis || classifyAxialCut(cutDirection, *axis)
                    == AxialCutAlignment::Unsupported)
                    throw std::runtime_error{
                        "This Cut Extrude orientation is not supported yet. "
                        "Create the sketch on a supported Extrude start/end face "
                        "and cut normal to that face."};
                const auto& targetRecord = find(data.targetResultFeatureId);
                if (targetRecord.kind() != FeatureKind::Extrude)
                    throw std::runtime_error{
                        "This Cut Extrude target cannot be reconstructed as a supported axial extrusion"};
                const auto& target = std::get<ExtrudeFeature>(targetRecord.data());
                const auto& targetSketch = std::get<SketchFeatureCache>(
                    output(target.sourceSketchFeatureId)).solvedSketch;
                const auto targetRegions = extractPlanarRegions(targetSketch);
                const auto cutterRegions = extractPlanarRegions(sketchCache.solvedSketch);
                auto subtraction = subtractAxialRegions(targetRegions, targetSketch.plane(),
                    target.distance, cutterRegions, sketchCache.solvedSketch.plane(),
                    cutDirection, data.distance);
                if (subtraction.status == AxialSubtractionStatus::Success)
                {
                    SolidFeatureCache cache{std::move(subtraction.solids)};
                    if (cache.solids.size() == 1)
                    {
                        const auto& targetPlane = targetSketch.plane();
                        const auto targetEnd = targetPlane.origin()
                            + targetPlane.normal() * target.distance;
                        addSemanticFaces(cache, ExtrudeCapRole::InheritedStartCap,
                            geometry::Plane{targetPlane.origin(), targetPlane.normal()},
                            sketch::SketchPlane{targetPlane.origin(), targetPlane.xAxis(),
                                targetPlane.yAxis() * -1.0});
                        addSemanticFaces(cache, ExtrudeCapRole::InheritedEndCap,
                            geometry::Plane{targetEnd, targetPlane.normal()},
                            sketch::SketchPlane{targetEnd, targetPlane.xAxis(), targetPlane.yAxis()});
                        const auto& cutterPlane = sketchCache.solvedSketch.plane();
                        const auto floorOrigin = cutterPlane.origin() + cutDirection * data.distance;
                        const auto floorOffset = math::dot(
                            floorOrigin - targetPlane.origin(), targetPlane.normal());
                        if (floorOffset > geometry::defaultGeometricTolerance
                            && floorOffset < target.distance - geometry::defaultGeometricTolerance)
                            addSemanticFaces(cache, ExtrudeCapRole::PocketFloor,
                                geometry::Plane{floorOrigin, cutterPlane.normal()},
                                sketch::SketchPlane{floorOrigin, cutterPlane.xAxis(), cutterPlane.yAxis()});
                    }
                    candidates[record.id().value()] = std::move(cache);
                }
                else if (subtraction.status == AxialSubtractionStatus::SuccessEmpty)
                    candidates[record.id().value()] = EmptyFeatureCache{};
                else if (subtraction.status == AxialSubtractionStatus::NoIntersection)
                    throw std::runtime_error{
                        "Cut profile does not intersect the target solid."};
                else if (subtraction.status == AxialSubtractionStatus::UnsupportedTarget)
                    throw std::runtime_error{
                        "Cut Extrude supports exact face-normal cuts on current Extrude results. "
                        + subtraction.diagnostic};
                else
                    throw std::runtime_error{"Cut Extrude failed: " + subtraction.diagnostic};
            }
        }
        catch (const std::exception& error)
        {
            failure = record.id();
            diagnostic = error.what();
            record.state_ = FeatureState::Failed;
            record.diagnostic_ = diagnostic;
        }
    }
    if (failure) return {RebuildStatus::Failed, failure, blocked, diagnostic};
    for (auto& record : records_)
        if (affected[record.id().value()])
        {
            record.cache_ = std::move(candidates[record.id().value()]);
            record.state_ = FeatureState::Clean;
            record.diagnostic_.clear();
        }
    if (resultFeatureId_.isValid() && find(resultFeatureId_).cache()
        && std::holds_alternative<EmptyFeatureCache>(*find(resultFeatureId_).cache()))
        return {RebuildStatus::SuccessEmpty, {}, 0, {}};
    return {};
}

FeatureModelCheckpoint FeatureModel::checkpoint() const
{
    FeatureModelCheckpoint result;
    result.resultFeatureId = resultFeatureId_;
    result.nextFeatureId = nextFeatureId_;
    result.features.reserve(records_.size());
    for (const auto& record : records_)
        result.features.push_back({record.id(), record.kind(), record.displayName(), record.data()});
    return result;
}

void FeatureModel::restoreCheckpoint(const FeatureModelCheckpoint& checkpoint)
{
    auto restored = restore(checkpoint.features, checkpoint.resultFeatureId,
        checkpoint.nextFeatureId, solver_);
    *this = std::move(restored);
}

FeatureModel FeatureModel::restore(std::vector<PersistedFeature> features,
                                   FeatureId resultFeatureId,
                                   FeatureId::Value nextFeatureId,
                                   constraints::ConstraintSolver solver)
{
    if (nextFeatureId == FeatureId::invalidValue)
        throw std::invalid_argument{"Invalid persisted FeatureId counter"};
    FeatureModel result{std::move(solver)};
    for (std::size_t index = 0; index < features.size(); ++index)
    {
        auto& feature = features[index];
        if (!feature.id.isValid() || feature.id.value() >= nextFeatureId
            || (index && feature.id <= features[index - 1].id) || feature.displayName.empty())
            throw std::invalid_argument{"Invalid persisted feature identity or name"};
        const auto dataKind = std::visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, SketchFeature>) return FeatureKind::Sketch;
            if constexpr (std::is_same_v<T, ExtrudeFeature>) return FeatureKind::Extrude;
            if constexpr (std::is_same_v<T, BooleanFeature>) return FeatureKind::Boolean;
            return FeatureKind::CutExtrude;
        }, feature.data);
        if (dataKind != feature.kind) throw std::invalid_argument{"Feature kind/data mismatch"};
        for (const auto dependency : FeatureRecord{feature.id, feature.kind,
                 feature.displayName, feature.data}.dependencies())
            if (!dependency.isValid() || dependency >= feature.id
                || std::none_of(features.begin(), features.begin() + static_cast<std::ptrdiff_t>(index),
                    [&](const PersistedFeature& candidate) { return candidate.id == dependency; }))
                throw std::invalid_argument{"Feature dependency must reference an earlier feature"};
        if (feature.kind == FeatureKind::Extrude)
        {
            const auto& data = std::get<ExtrudeFeature>(feature.data);
            if (!std::isfinite(data.distance) || data.distance <= geometry::defaultGeometricTolerance
                || std::find_if(features.begin(), features.end(), [&](const PersistedFeature& candidate) {
                    return candidate.id == data.sourceSketchFeatureId;
                })->kind != FeatureKind::Sketch)
                throw std::invalid_argument{"Invalid persisted ExtrudeFeature"};
        }
        if (feature.kind == FeatureKind::Boolean)
        {
            const auto& data = std::get<BooleanFeature>(feature.data);
            const auto isSolid = [&](FeatureId id) {
                const auto found = std::find_if(features.begin(), features.end(),
                    [&](const PersistedFeature& candidate) { return candidate.id == id; });
                if (found == features.end()) return false;
                const auto kind = found->kind;
                return kind == FeatureKind::Extrude || kind == FeatureKind::Boolean;
            };
            if (!isSolid(data.inputA) || !isSolid(data.inputB))
                throw std::invalid_argument{"Invalid persisted BooleanFeature inputs"};
        }
        if (feature.kind == FeatureKind::CutExtrude)
        {
            const auto& data = std::get<CutExtrudeFeature>(feature.data);
            if (!std::isfinite(data.distance) || data.distance <= geometry::defaultGeometricTolerance
                || std::find_if(features.begin(), features.end(), [&](const PersistedFeature& candidate) {
                    return candidate.id == data.sourceSketchFeatureId;
                })->kind != FeatureKind::Sketch)
                throw std::invalid_argument{"Invalid persisted CutExtrudeFeature"};
            const auto target = std::find_if(features.begin(), features.end(),
                [&](const PersistedFeature& candidate) { return candidate.id == data.targetResultFeatureId; });
            if (target == features.end()) throw std::invalid_argument{"Invalid CutExtrude target"};
            const auto targetKind = target->kind;
            if (targetKind != FeatureKind::Extrude && targetKind != FeatureKind::Boolean
                && targetKind != FeatureKind::CutExtrude)
                throw std::invalid_argument{"Invalid CutExtrude target"};
        }
        result.records_.push_back(FeatureRecord{feature.id, feature.kind,
            std::move(feature.displayName), std::move(feature.data)});
    }
    result.nextFeatureId_ = nextFeatureId;
    if (resultFeatureId.isValid())
    {
        if (!result.tryFind(resultFeatureId) || !result.isSolidProducer(resultFeatureId))
            throw std::invalid_argument{"Invalid persisted resultFeatureId"};
        result.resultFeatureId_ = resultFeatureId;
    }
    return result;
}
}
