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

namespace microsw::features
{
enum class RegionBooleanStatus
{
    Success,
    Empty,
    Ambiguous,
    NumericalFailure
};

struct RegionBooleanResult
{
    RegionBooleanStatus status{RegionBooleanStatus::NumericalFailure};
    PlanarRegionSet regions;
    std::string diagnostic;
    [[nodiscard]] bool succeeded() const noexcept
    { return status == RegionBooleanStatus::Success || status == RegionBooleanStatus::Empty; }
};

[[nodiscard]] RegionBooleanResult subtractPlanarRegions(
    const PlanarRegionSet& target,
    const PlanarRegionSet& cutter);

[[nodiscard]] RegionBooleanResult intersectPlanarRegions(
    const PlanarRegionSet& first,
    const PlanarRegionSet& second);

[[nodiscard]] RegionBooleanResult partitionPlanarRegions(
    const PlanarRegionSet& regions,
    const PlanarRegionSet& splitter);
}
