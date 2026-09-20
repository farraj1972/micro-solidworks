#pragma once

#include "core/topology/Solid.h"
#include "modeling/Profile.h"

namespace microsw::modeling
{
[[nodiscard]] topology::Solid extrude(const Profile& profile, math::Scalar distance);
}
