#pragma once

#include "core/sketch/SketchConstraint.h"
#include "core/sketch/SketchConstraintId.h"
#include "core/sketch/SketchEntityId.h"
#include "core/sketch/Sketch.h"
#include "features/FeatureId.h"
#include "interaction/TopologyTarget.h"

#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace microsw::interaction
{

enum class EditorCommand
{
    Select,
    Line,
    Rectangle,
    Circle,
    Dimension,
    Horizontal,
    Vertical,
    Coincident,
    Extrude,
    CutExtrude
};

enum class ToolStep { Idle, PickFirst, PickSecond, EditProperties, Preview };
enum class DisplayState { Visible, Hidden, ActiveEditing, ContextReference };

using LogicalTarget = std::variant<features::FeatureId, sketch::SketchEntityId,
    sketch::SketchElementRef, sketch::SketchConstraintId,
    TopologyFaceTarget, TopologyEdgeTarget>;

[[nodiscard]] std::optional<features::FeatureId> owningFeature(const LogicalTarget& target) noexcept;
[[nodiscard]] bool isTopologyTarget(const LogicalTarget& target) noexcept;

enum class SnapKind { Endpoint, Midpoint, Grid };
struct SnapCandidate
{
    SnapKind kind;
    geometry::Point2 point;
    std::optional<sketch::SketchElementRef> element;
};

enum class InferenceKind { Horizontal, Vertical, Coincident };
struct InferenceCandidate
{
    InferenceKind kind;
    geometry::Point2 point;
    std::optional<sketch::SketchElementRef> element;
};

struct PropertyEditContext
{
    enum class Kind { ExtrudeDistance, CutExtrudeDepth, DrivingDimension };
    Kind kind;
    math::Scalar value;
};

struct ScreenPoint
{
    math::Scalar x;
    math::Scalar y;
};

class EditorContext
{
public:
    void activateSketch(features::FeatureId id);
    void exitSketch() noexcept;
    [[nodiscard]] std::optional<features::FeatureId> activeFeatureId() const noexcept;
    [[nodiscard]] std::optional<features::FeatureId> activeSketchFeatureId() const noexcept;

    void activateCommand(EditorCommand command) noexcept;
    void cancelStep() noexcept;
    void escape() noexcept;
    [[nodiscard]] EditorCommand activeCommand() const noexcept { return command_; }
    [[nodiscard]] ToolStep toolStep() const noexcept { return step_; }

    void setHover(std::optional<LogicalTarget> target) { hover_ = std::move(target); }
    [[nodiscard]] const std::optional<LogicalTarget>& hover() const noexcept { return hover_; }
    void select(std::optional<LogicalTarget> target);
    [[nodiscard]] const std::optional<LogicalTarget>& selection() const noexcept { return selection_; }

    void addOperand(sketch::SketchElementRef operand);
    void clearOperands() noexcept { operands_.clear(); }
    [[nodiscard]] const std::vector<sketch::SketchElementRef>& operands() const noexcept { return operands_; }

    void setSnapCandidate(std::optional<SnapCandidate> candidate) { snap_ = std::move(candidate); }
    void setInferenceCandidate(std::optional<InferenceCandidate> candidate) { inference_ = std::move(candidate); }
    [[nodiscard]] const std::optional<SnapCandidate>& snapCandidate() const noexcept { return snap_; }
    [[nodiscard]] const std::optional<InferenceCandidate>& inferenceCandidate() const noexcept { return inference_; }

    void beginPreview(ScreenPoint anchor) noexcept { previewAnchor_ = anchor; previewCursor_ = anchor; step_ = ToolStep::PickSecond; }
    void updatePreview(ScreenPoint cursor) noexcept { if (previewAnchor_) previewCursor_ = cursor; }
    void clearPreview() noexcept { previewAnchor_.reset(); previewCursor_.reset(); previewCurve_.clear(); }
    void setPreviewCurve(std::vector<ScreenPoint> curve) { previewCurve_ = std::move(curve); }
    [[nodiscard]] const std::optional<ScreenPoint>& previewAnchor() const noexcept { return previewAnchor_; }
    [[nodiscard]] const std::optional<ScreenPoint>& previewCursor() const noexcept { return previewCursor_; }
    [[nodiscard]] const std::vector<ScreenPoint>& previewCurve() const noexcept { return previewCurve_; }

    void setDisplayState(features::FeatureId id, DisplayState state);
    [[nodiscard]] DisplayState displayState(features::FeatureId id) const noexcept;
    void beginPropertyEdit(PropertyEditContext context) { propertyEdit_ = context; step_ = ToolStep::EditProperties; }
    void clearPropertyEdit() noexcept { propertyEdit_.reset(); }
    [[nodiscard]] const std::optional<PropertyEditContext>& propertyEdit() const noexcept { return propertyEdit_; }

    [[nodiscard]] bool commandAvailable(EditorCommand command, bool hasClosedSketch) const noexcept;
    void resetSession() noexcept;
    void clearTopologySelectionOnRebuild() noexcept;
    void reconcileDeletedFeatures(const std::vector<features::FeatureId>& deleted) noexcept;

private:
    std::optional<features::FeatureId> activeFeature_;
    std::optional<features::FeatureId> activeSketch_;
    EditorCommand command_{EditorCommand::Select};
    ToolStep step_{ToolStep::Idle};
    std::optional<LogicalTarget> hover_;
    std::optional<LogicalTarget> selection_;
    std::vector<sketch::SketchElementRef> operands_;
    std::optional<SnapCandidate> snap_;
    std::optional<InferenceCandidate> inference_;
    std::optional<ScreenPoint> previewAnchor_;
    std::optional<ScreenPoint> previewCursor_;
    std::vector<ScreenPoint> previewCurve_;
    std::map<features::FeatureId, DisplayState> display_;
    std::optional<PropertyEditContext> propertyEdit_;
};

[[nodiscard]] std::optional<SnapCandidate> resolveSnap(
    const sketch::Sketch& sketch, const geometry::Point2& cursor,
    math::Scalar tolerance, math::Scalar gridSpacing, bool gridEnabled);

[[nodiscard]] std::optional<InferenceCandidate> resolveInference(
    const geometry::Point2& anchor, const geometry::Point2& cursor,
    math::Scalar tolerance, const std::optional<SnapCandidate>& snap);

[[nodiscard]] std::optional<sketch::SketchElementRef> nearestElement(
    const sketch::Sketch& sketch, sketch::SketchEntityId entity,
    const geometry::Point2& cursor, math::Scalar tolerance);

}
