#pragma once

#include "constraints/ConstraintSolver.h"
#include "features/FeatureId.h"
#include "modeling/Boolean.h"

#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace microsw::features
{
enum class FeatureKind { Sketch, Extrude, Boolean, CutExtrude };
enum class FeatureState { Clean, Dirty, Failed, BlockedByDependency };
enum class RebuildStatus { Success, SuccessEmpty, Failed };

enum class ExtrudeCapRole
{
    StartCap,
    EndCap,
    InheritedStartCap,
    InheritedEndCap,
    PocketFloor
};
[[nodiscard]] const char* semanticFaceRoleName(ExtrudeCapRole role) noexcept;
struct ExplicitPlaneSupport {};
struct SemanticFaceSupport { FeatureId sourceFeatureId; ExtrudeCapRole capRole; };
using SketchSupport = std::variant<ExplicitPlaneSupport, SemanticFaceSupport>;
struct SketchFeature { sketch::Sketch sketch; SketchSupport support{ExplicitPlaneSupport{}}; };
struct ExtrudeFeature { FeatureId sourceSketchFeatureId; math::Scalar distance; };
enum class CutDirection { Forward, Reverse };
struct CutExtrudeFeature
{
    FeatureId sourceSketchFeatureId;
    FeatureId targetResultFeatureId;
    math::Scalar distance;
    CutDirection direction{CutDirection::Forward};
};
struct BooleanFeature { FeatureId inputA; FeatureId inputB; modeling::BooleanOperation operation; };
using FeatureData = std::variant<SketchFeature, ExtrudeFeature, BooleanFeature, CutExtrudeFeature>;

struct SketchFeatureCache { sketch::Sketch solvedSketch; };
struct SemanticFaceProvenance
{
    ExtrudeCapRole role;
    std::size_t bodyIndex;
    topology::FaceId faceId;
    sketch::SketchPlane sketchPlane;
};
struct SolidFeatureCache
{
    std::vector<topology::Solid> solids;
    std::vector<SemanticFaceProvenance> semanticFaces;
};
[[nodiscard]] const sketch::SketchPlane& resolveSemanticFacePlane(
    const SolidFeatureCache& cache, ExtrudeCapRole role);
struct EmptyFeatureCache {};
using FeatureCache = std::variant<SketchFeatureCache, SolidFeatureCache, EmptyFeatureCache>;

class FeatureRecord
{
public:
    [[nodiscard]] FeatureId id() const noexcept { return id_; }
    [[nodiscard]] FeatureKind kind() const noexcept { return kind_; }
    [[nodiscard]] const std::string& displayName() const noexcept { return displayName_; }
    [[nodiscard]] FeatureState state() const noexcept { return state_; }
    [[nodiscard]] const std::string& diagnostic() const noexcept { return diagnostic_; }
    [[nodiscard]] const FeatureData& data() const noexcept { return data_; }
    [[nodiscard]] const std::optional<FeatureCache>& cache() const noexcept { return cache_; }
    [[nodiscard]] std::vector<FeatureId> dependencies() const;
private:
    friend class FeatureModel;
    FeatureRecord(FeatureId id, FeatureKind kind, std::string name, FeatureData data)
        : id_{id}, kind_{kind}, displayName_{std::move(name)}, data_{std::move(data)} {}
    FeatureId id_;
    FeatureKind kind_;
    std::string displayName_;
    FeatureState state_{FeatureState::Dirty};
    std::string diagnostic_;
    FeatureData data_;
    std::optional<FeatureCache> cache_;
};

struct RebuildResult
{
    RebuildStatus status{RebuildStatus::Success};
    std::optional<FeatureId> failedFeature;
    std::size_t blockedCount{};
    std::string diagnostic;
};

struct PersistedFeature
{
    FeatureId id;
    FeatureKind kind;
    std::string displayName;
    FeatureData data;
};

// Authoritative document state only. Rebuild state, diagnostics and derived
// caches are deliberately excluded and are regenerated after restoration.
struct FeatureModelCheckpoint
{
    std::vector<PersistedFeature> features;
    FeatureId resultFeatureId;
    FeatureId::Value nextFeatureId{};
};

class FeatureModel
{
public:
    using SketchEdit = std::function<void(sketch::Sketch&)>;
    FeatureModel();
    explicit FeatureModel(constraints::ConstraintSolver solver);
    FeatureId addSketch(sketch::Sketch sketch);
    FeatureId addFaceSketch(sketch::Sketch sketch, FeatureId sourceExtrude, ExtrudeCapRole role);
    FeatureId addExtrude(FeatureId sketchId, math::Scalar distance);
    FeatureId addCutExtrude(FeatureId sketchId, FeatureId targetResultId,
        math::Scalar distance, CutDirection direction = CutDirection::Forward);
    FeatureId addBoolean(FeatureId inputA, FeatureId inputB, modeling::BooleanOperation operation);
    void editSketch(FeatureId id, const SketchEdit& edit);
    void setExtrudeDistance(FeatureId id, math::Scalar distance);
    void setCutExtrude(FeatureId id, math::Scalar distance, CutDirection direction);
    void setBooleanOperation(FeatureId id, modeling::BooleanOperation operation);
    void renameFeature(FeatureId id, std::string name);
    [[nodiscard]] std::vector<FeatureId> deletionClosure(FeatureId id) const;
    [[nodiscard]] std::vector<FeatureId> deleteFeature(FeatureId id);
    void setResultFeatureId(FeatureId id);
    [[nodiscard]] FeatureId resultFeatureId() const noexcept { return resultFeatureId_; }
    [[nodiscard]] const std::vector<FeatureRecord>& history() const noexcept { return records_; }
    [[nodiscard]] const FeatureRecord& find(FeatureId id) const;
    [[nodiscard]] std::optional<ExtrudeCapRole> semanticFaceRole(
        FeatureId source, std::size_t bodyIndex, topology::FaceId faceId) const;
    [[nodiscard]] RebuildResult rebuild();
    [[nodiscard]] FeatureId::Value nextFeatureId() const noexcept { return nextFeatureId_; }
    [[nodiscard]] FeatureModelCheckpoint checkpoint() const;
    void restoreCheckpoint(const FeatureModelCheckpoint& checkpoint);
    [[nodiscard]] static FeatureModel restore(std::vector<PersistedFeature> features,
        FeatureId resultFeatureId, FeatureId::Value nextFeatureId,
        constraints::ConstraintSolver solver = constraints::ConstraintSolver{constraints::SolverPolicy{}});
private:
    FeatureRecord& mutableFind(FeatureId id);
    [[nodiscard]] const FeatureRecord* tryFind(FeatureId id) const noexcept;
    void markDirtyClosure(FeatureId id);
    [[nodiscard]] bool isSolidProducer(FeatureId id) const;
    [[nodiscard]] std::string nextName(FeatureKind kind) const;
    constraints::ConstraintSolver solver_;
    std::vector<FeatureRecord> records_;
    FeatureId resultFeatureId_;
    FeatureId::Value nextFeatureId_{};
};
}
