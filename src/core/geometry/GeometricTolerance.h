#pragma once

#include "core/math/Scalar.h"

namespace microsw::geometry
{

// Kernel length tolerance in document units (initially mm), not B1's numeric
// comparison tolerance or a guarantee of physical accuracy.
inline constexpr math::Scalar defaultGeometricTolerance = 1.0e-9;

}
