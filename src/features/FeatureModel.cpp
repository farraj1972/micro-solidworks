#include "features/FeatureModel.h"

#include "core/geometry/GeometricTolerance.h"
#include "features/SketchProfileAdapter.h"
#include "modeling/Extrusion.h"

#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace microsw::features
{
std::vector<FeatureId> FeatureRecord::dependencies() const
{
    return std::visit([](const auto& value) -> std::vector<FeatureId> {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, SketchFeature>) return {};
        else if constexpr (std::is_same_v<T, ExtrudeFeature>) return {value.sourceSketchFeatureId};
        else return {value.inputA, value.inputB};
    }, data_);
}

FeatureModel::FeatureModel() : solver_{constraints::SolverPolicy{}} {}
FeatureModel::FeatureModel(constraints::ConstraintSolver solver) : solver_{std::move(solver)} {}

std::string FeatureModel::nextName(FeatureKind kind) const
{
    std::size_t count{};
    for (const auto& record : records_) if (record.kind() == kind) ++count;
    const char* prefix = kind == FeatureKind::Sketch ? "Sketch" : kind == FeatureKind::Extrude ? "Extrude" : "Boolean";
    return prefix + std::to_string(count + 1);
}

FeatureId FeatureModel::addSketch(sketch::Sketch sketch)
{
    const auto id = FeatureId{nextFeatureId_};
    if (!id.isValid()) throw std::overflow_error{"Feature ID space exhausted"};
    records_.push_back(FeatureRecord{id, FeatureKind::Sketch, nextName(FeatureKind::Sketch), SketchFeature{std::move(sketch)}});
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
    if (!id.isValid() || id.value() >= records_.size()) throw std::out_of_range{"Invalid FeatureId"};
    return records_[id.value()];
}
FeatureRecord& FeatureModel::mutableFind(FeatureId id)
{
    return const_cast<FeatureRecord&>(std::as_const(*this).find(id));
}
bool FeatureModel::isSolidProducer(FeatureId id) const
{
    const auto kind = find(id).kind();
    return kind == FeatureKind::Extrude || kind == FeatureKind::Boolean;
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

RebuildResult FeatureModel::rebuild()
{
    std::vector<std::optional<FeatureCache>> candidates(records_.size());
    std::vector<bool> affected(records_.size());
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
                auto solved = std::get<SketchFeature>(record.data_).sketch;
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
                candidates[record.id().value()] = SolidFeatureCache{modeling::extrude(extractProfile(sketchCache.solvedSketch), data.distance)};
            }
            else
            {
                const auto& data = std::get<BooleanFeature>(record.data_);
                const auto& a = std::get<SolidFeatureCache>(output(data.inputA)).solid;
                const auto& b = std::get<SolidFeatureCache>(output(data.inputB)).solid;
                auto result = modeling::booleanOperation(a, b, data.operation);
                if (result.status() == modeling::BooleanStatus::SUCCESS)
                    candidates[record.id().value()] = SolidFeatureCache{*result.solid()};
                else if (result.status() == modeling::BooleanStatus::EMPTY)
                    candidates[record.id().value()] = EmptyFeatureCache{};
                else throw std::runtime_error{"Boolean rebuild failed"};
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

FeatureModel FeatureModel::restore(std::vector<PersistedFeature> features,
                                   FeatureId resultFeatureId,
                                   FeatureId::Value nextFeatureId,
                                   constraints::ConstraintSolver solver)
{
    if (nextFeatureId == FeatureId::invalidValue || nextFeatureId != features.size())
        throw std::invalid_argument{"Invalid persisted FeatureId counter"};
    FeatureModel result{std::move(solver)};
    for (std::size_t index = 0; index < features.size(); ++index)
    {
        auto& feature = features[index];
        if (!feature.id.isValid() || feature.id.value() != index || feature.displayName.empty())
            throw std::invalid_argument{"Invalid persisted feature identity or name"};
        const auto dataKind = std::visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, SketchFeature>) return FeatureKind::Sketch;
            if constexpr (std::is_same_v<T, ExtrudeFeature>) return FeatureKind::Extrude;
            return FeatureKind::Boolean;
        }, feature.data);
        if (dataKind != feature.kind) throw std::invalid_argument{"Feature kind/data mismatch"};
        for (const auto dependency : FeatureRecord{feature.id, feature.kind,
                 feature.displayName, feature.data}.dependencies())
            if (!dependency.isValid() || dependency.value() >= index)
                throw std::invalid_argument{"Feature dependency must reference an earlier feature"};
        if (feature.kind == FeatureKind::Extrude)
        {
            const auto& data = std::get<ExtrudeFeature>(feature.data);
            if (!std::isfinite(data.distance) || data.distance <= geometry::defaultGeometricTolerance
                || features[data.sourceSketchFeatureId.value()].kind != FeatureKind::Sketch)
                throw std::invalid_argument{"Invalid persisted ExtrudeFeature"};
        }
        if (feature.kind == FeatureKind::Boolean)
        {
            const auto& data = std::get<BooleanFeature>(feature.data);
            const auto isSolid = [&](FeatureId id) {
                const auto kind = features[id.value()].kind;
                return kind == FeatureKind::Extrude || kind == FeatureKind::Boolean;
            };
            if (!isSolid(data.inputA) || !isSolid(data.inputB))
                throw std::invalid_argument{"Invalid persisted BooleanFeature inputs"};
        }
        result.records_.push_back(FeatureRecord{feature.id, feature.kind,
            std::move(feature.displayName), std::move(feature.data)});
    }
    result.nextFeatureId_ = nextFeatureId;
    if (resultFeatureId.isValid())
    {
        if (resultFeatureId.value() >= result.records_.size()
            || !result.isSolidProducer(resultFeatureId))
            throw std::invalid_argument{"Invalid persisted resultFeatureId"};
        result.resultFeatureId_ = resultFeatureId;
    }
    return result;
}
}
