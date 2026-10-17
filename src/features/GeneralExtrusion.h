#pragma once

#include "core/sketch/SketchPlane.h"
#include "core/topology/Solid.h"
#include "features/PlanarRegion.h"

#include <string>
#include <vector>

namespace microsw::features
{
enum class GeneralExtrusionStatus
{
    Success,
    InvalidRegions,
    InvalidDistance,
    TopologyFailure
};

struct GeneralExtrusionResult
{
    GeneralExtrusionStatus status{GeneralExtrusionStatus::TopologyFailure};
    std::vector<topology::Solid> solids;
    std::string diagnostic;

    [[nodiscard]] bool succeeded() const noexcept
    {
        return status == GeneralExtrusionStatus::Success;
    }
};

[[nodiscard]] GeneralExtrusionResult extrudeRegions(
    const PlanarRegionSet& regions,
    const sketch::SketchPlane& plane,
    math::Scalar distance);
}
