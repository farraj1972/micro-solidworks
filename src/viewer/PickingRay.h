#pragma once

#include "core/geometry/Ray3.h"
#include "viewer/PickingContext.h"

namespace microsw::viewer
{
// Perspective: eye origin, direction through the mouse on the view frustum.
// Orthographic: image-plane offset from the eye, parallel to camera.forward().
// Returns a unit, scene-facing ray without a matrix inverse. Outside-viewport
// finite coordinates extrapolate the ray; pickGeometry separately gates hits.
[[nodiscard]] geometry::Ray3 makePickingRay(const PickingContext& context);
}
