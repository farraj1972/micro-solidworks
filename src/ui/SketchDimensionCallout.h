#pragma once

#include "core/sketch/Sketch.h"
#include "interaction/EditorContext.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace microsw::ui
{

// Ephemeral screen geometry derived from the solved Sketch and current camera.
// IDs are Sketch-owned logical constraint IDs, never presentation identities.
struct SketchDimensionCallout
{
    sketch::SketchConstraintId constraint;
    sketch::SketchEntityId entity;
    interaction::ScreenPoint first;
    interaction::ScreenPoint second;
    interaction::ScreenPoint label;
    std::string text;
};

using SketchPointProjector = std::function<std::optional<interaction::ScreenPoint>(
    const geometry::Point2&)>;

[[nodiscard]] std::vector<SketchDimensionCallout> deriveSketchDimensionCallouts(
    const sketch::Sketch& solvedSketch, const SketchPointProjector& project);

[[nodiscard]] std::optional<sketch::SketchConstraintId> hitTestSketchDimensionCallouts(
    const std::vector<SketchDimensionCallout>& callouts,
    interaction::ScreenPoint pointer,
    const std::function<double(const std::string&)>& textWidth,
    double textHeight);

}
