#pragma once

#include "core/topology/Solid.h"

namespace microsw::topology
{

// Builds the deterministic convex B7 validation solid, centered at the origin.
[[nodiscard]] Solid makeCuboid(
    math::Scalar width, math::Scalar height, math::Scalar depth);

}
