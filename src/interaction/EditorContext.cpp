#include "interaction/EditorContext.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace microsw::interaction
{
namespace
{
math::Scalar distance(const geometry::Point2& a, const geometry::Point2& b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}
}

void EditorContext::activateSketch(features::FeatureId id)
{
    activeFeature_ = id;
    activeSketch_ = id;
    display_[id] = DisplayState::ActiveEditing;
    activateCommand(EditorCommand::Select);
}

void EditorContext::exitSketch() noexcept
{
    if (activeSketch_) display_[*activeSketch_] = DisplayState::Visible;
    activeSketch_.reset();
    activateCommand(EditorCommand::Select);
}

std::optional<features::FeatureId> EditorContext::activeFeatureId() const noexcept { return activeFeature_; }
std::optional<features::FeatureId> EditorContext::activeSketchFeatureId() const noexcept { return activeSketch_; }

void EditorContext::activateCommand(EditorCommand command) noexcept
{
    command_ = command;
    operands_.clear();
    snap_.reset();
    inference_.reset();
    clearPreview();
    propertyEdit_.reset();
    step_ = command == EditorCommand::Select ? ToolStep::Idle : ToolStep::PickFirst;
}

void EditorContext::cancelStep() noexcept
{
    operands_.clear();
    snap_.reset();
    inference_.reset();
    clearPreview();
    propertyEdit_.reset();
    step_ = command_ == EditorCommand::Select ? ToolStep::Idle : ToolStep::PickFirst;
}

void EditorContext::escape() noexcept
{
    if (step_ != ToolStep::PickFirst && step_ != ToolStep::Idle) cancelStep();
    else activateCommand(EditorCommand::Select);
}

void EditorContext::select(std::optional<LogicalTarget> target)
{
    selection_ = std::move(target);
    if (selection_)
        if (const auto* feature = std::get_if<features::FeatureId>(&*selection_)) activeFeature_ = *feature;
}

void EditorContext::addOperand(sketch::SketchElementRef operand)
{
    if (std::find(operands_.begin(), operands_.end(), operand) == operands_.end()) operands_.push_back(operand);
    step_ = operands_.size() < 2 ? ToolStep::PickSecond : ToolStep::Preview;
}

void EditorContext::setDisplayState(features::FeatureId id, DisplayState state) { display_[id] = state; }
DisplayState EditorContext::displayState(features::FeatureId id) const noexcept
{
    const auto found = display_.find(id);
    return found == display_.end() ? DisplayState::Visible : found->second;
}

bool EditorContext::commandAvailable(EditorCommand command, bool hasClosedSketch) const noexcept
{
    switch (command)
    {
    case EditorCommand::Select: return true;
    case EditorCommand::Line:
    case EditorCommand::Rectangle:
    case EditorCommand::Circle:
    case EditorCommand::Dimension:
    case EditorCommand::Horizontal:
    case EditorCommand::Vertical:
    case EditorCommand::Coincident: return activeSketch_.has_value();
    case EditorCommand::Extrude: return hasClosedSketch && (activeSketch_.has_value()
        || (selection_ && std::holds_alternative<features::FeatureId>(*selection_)));
    }
    return false;
}

void EditorContext::resetSession() noexcept
{
    activeFeature_.reset(); activeSketch_.reset(); hover_.reset(); selection_.reset();
    operands_.clear(); snap_.reset(); inference_.reset(); clearPreview(); display_.clear(); propertyEdit_.reset();
    command_ = EditorCommand::Select; step_ = ToolStep::Idle;
}

std::optional<SnapCandidate> resolveSnap(const sketch::Sketch& sketch,
    const geometry::Point2& cursor, math::Scalar tolerance,
    math::Scalar gridSpacing, bool gridEnabled)
{
    if (!std::isfinite(tolerance) || tolerance <= 0 || !std::isfinite(gridSpacing) || gridSpacing <= 0)
        throw std::invalid_argument{"Snap tolerance and grid spacing must be finite and positive"};
    std::optional<SnapCandidate> endpoint;
    std::optional<SnapCandidate> midpoint;
    math::Scalar endpointDistance = std::numeric_limits<math::Scalar>::infinity();
    math::Scalar midpointDistance = endpointDistance;
    const auto consider = [&](SnapCandidate candidate, math::Scalar& bestDistance,
                              std::optional<SnapCandidate>& best)
    {
        const auto d = distance(cursor, candidate.point);
        if (d <= tolerance && d < bestDistance) { bestDistance = d; best = std::move(candidate); }
    };
    for (const auto id : sketch.entityIds())
    {
        const auto& entity = sketch.find(id);
        std::visit([&](const auto& typed)
        {
            using T = std::decay_t<decltype(typed)>;
            if constexpr (std::is_same_v<T, sketch::SketchLine>)
            {
                consider({SnapKind::Endpoint, typed.geometry.a(), sketch::SketchElementRef{id, sketch::SubElementKind::LineStart}}, endpointDistance, endpoint);
                consider({SnapKind::Endpoint, typed.geometry.b(), sketch::SketchElementRef{id, sketch::SubElementKind::LineEnd}}, endpointDistance, endpoint);
                const geometry::Point2 mid{(typed.geometry.a().x() + typed.geometry.b().x()) / 2,
                    (typed.geometry.a().y() + typed.geometry.b().y()) / 2};
                consider({SnapKind::Midpoint, mid, sketch::SketchElementRef{id, sketch::SubElementKind::LineBody}}, midpointDistance, midpoint);
            }
            else if constexpr (std::is_same_v<T, sketch::SketchCircle>)
                consider({SnapKind::Endpoint, typed.geometry.center(), sketch::SketchElementRef{id, sketch::SubElementKind::CircleCenter}}, endpointDistance, endpoint);
        }, entity.geometry());
    }
    if (endpoint) return endpoint;
    if (midpoint) return midpoint;
    if (!gridEnabled) return std::nullopt;
    geometry::Point2 grid{std::round(cursor.x() / gridSpacing) * gridSpacing,
                          std::round(cursor.y() / gridSpacing) * gridSpacing};
    if (distance(cursor, grid) <= tolerance) return SnapCandidate{SnapKind::Grid, grid, std::nullopt};
    return std::nullopt;
}

std::optional<InferenceCandidate> resolveInference(const geometry::Point2& anchor,
    const geometry::Point2& cursor, math::Scalar tolerance,
    const std::optional<SnapCandidate>& snap)
{
    if (!std::isfinite(tolerance) || tolerance <= 0) throw std::invalid_argument{"Inference tolerance must be positive"};
    if (snap && snap->element)
        return InferenceCandidate{InferenceKind::Coincident, snap->point, snap->element};
    const auto dx = std::abs(cursor.x() - anchor.x());
    const auto dy = std::abs(cursor.y() - anchor.y());
    if (dy <= tolerance && dy <= dx) return InferenceCandidate{InferenceKind::Horizontal, {cursor.x(), anchor.y()}, std::nullopt};
    if (dx <= tolerance) return InferenceCandidate{InferenceKind::Vertical, {anchor.x(), cursor.y()}, std::nullopt};
    return std::nullopt;
}

std::optional<sketch::SketchElementRef> nearestElement(const sketch::Sketch& sketch,
    sketch::SketchEntityId entityId, const geometry::Point2& cursor, math::Scalar tolerance)
{
    if (!sketch.contains(entityId) || !std::isfinite(tolerance) || tolerance <= 0) return std::nullopt;
    const auto& entity = sketch.find(entityId);
    return std::visit([&](const auto& typed) -> std::optional<sketch::SketchElementRef>
    {
        using T = std::decay_t<decltype(typed)>;
        if constexpr (std::is_same_v<T, sketch::SketchLine>)
        {
            if (distance(cursor, typed.geometry.a()) <= tolerance) return sketch::SketchElementRef{entityId, sketch::SubElementKind::LineStart};
            if (distance(cursor, typed.geometry.b()) <= tolerance) return sketch::SketchElementRef{entityId, sketch::SubElementKind::LineEnd};
            return sketch::SketchElementRef{entityId, sketch::SubElementKind::LineBody};
        }
        else if constexpr (std::is_same_v<T, sketch::SketchCircle>)
            return sketch::SketchElementRef{entityId, distance(cursor, typed.geometry.center()) <= tolerance
                ? sketch::SubElementKind::CircleCenter : sketch::SubElementKind::CircleRadius};
        else return std::nullopt;
    }, entity.geometry());
}

}
