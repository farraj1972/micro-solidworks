#pragma once

#include "core/geometry/Point3.h"
#include "presentation/VisualEntityId.h"
#include "viewer/PickingContext.h"

#include <optional>

namespace microsw::presentation { class GeometryPresentation; }

namespace microsw::viewer
{
struct ScreenPoint
{
    math::Scalar x;
    math::Scalar y;
    math::Scalar depth; // Positive camera-space depth, not Euclidean eye distance.
};

// Pure projection to Workspace-local logical pixels. Points outside the six
// clip planes (including behind the eye) return nullopt. No GL state is read.
[[nodiscard]] std::optional<ScreenPoint> projectWorldToScreen(
    const geometry::Point3& point, const PickingContext& context);

struct PickHit
{
    presentation::VisualEntityId id;
    math::Scalar screenDistance; // Logical pixels.
};

// Point3, Segment3, and the current PresentedLines finite representation only.
// Mouse bounds are half-open [0,width) x [0,height). Segments are frustum-clipped.
// Ranking: smaller pixel distance; within 1e-7 px, smaller camera-space depth;
// numerically equal depth retains insertion order. No primitive-type priority.
// Exact occlusion is deferred: no depth-buffer reads, hover or selection state.
[[nodiscard]] std::optional<PickHit> pickGeometry(
    const presentation::GeometryPresentation& presentation, const PickingContext& context);
}
