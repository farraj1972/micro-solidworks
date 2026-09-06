#pragma once

#include "core/math/Vector3.h"
#include "viewer/VisualState.h"

#include <vector>

namespace microsw::presentation { class GeometryPresentation; }

namespace microsw::viewer
{
[[nodiscard]] std::vector<math::Vector3> presentedSegmentVertices(
    const presentation::GeometryPresentation& presentation,
    std::optional<VisualStateFilter> filter = std::nullopt);
}
