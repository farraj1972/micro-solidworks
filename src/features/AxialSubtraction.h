#pragma once

#include "core/sketch/SketchPlane.h"
#include "core/topology/Solid.h"
#include "features/PlanarRegion.h"

#include <string>
#include <vector>

namespace microsw::features
{
enum class AxialSubtractionStatus
{
    Success,
    SuccessEmpty,
    NoIntersection,
    UnsupportedTarget,
    RegionFailure,
    TopologyFailure
};

struct AxialSubtractionResult
{
    AxialSubtractionStatus status{AxialSubtractionStatus::TopologyFailure};
    std::vector<topology::Solid> solids;
    std::string diagnostic;
    [[nodiscard]] bool succeeded() const noexcept
    {
        return status == AxialSubtractionStatus::Success
            || status == AxialSubtractionStatus::SuccessEmpty;
    }
};

[[nodiscard]] AxialSubtractionResult subtractAxialRegions(
    const PlanarRegionSet& targetRegions,
    const sketch::SketchPlane& targetPlane,
    math::Scalar targetDistance,
    const PlanarRegionSet& cutterRegions,
    const sketch::SketchPlane& cutterPlane,
    const math::Vector3& cutterDirection,
    math::Scalar cutterDistance);
}
