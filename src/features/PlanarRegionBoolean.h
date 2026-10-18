#pragma once

#include "features/PlanarRegion.h"

namespace microsw::features::detail
{

// Splits one exact region curve at points already known to lie on it. Endpoints
// are ignored, parameters are tolerance-deduplicated, and no polygonization is
// performed. A full circle is represented by exact CircularArc pieces.
[[nodiscard]] std::vector<RegionCurve2> splitRegionCurve(
    const RegionCurve2& curve,
    const std::vector<geometry::Point2>& points);

}
