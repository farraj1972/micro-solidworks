#pragma once

#include "core/math/Scalar.h"

namespace microsw::math
{

inline constexpr Scalar defaultAbsoluteTolerance = 1.0e-12;
inline constexpr Scalar defaultRelativeTolerance = 1.0e-12;

[[nodiscard]] bool almostEqual(
    Scalar first,
    Scalar second,
    Scalar absoluteTolerance = defaultAbsoluteTolerance,
    Scalar relativeTolerance = defaultRelativeTolerance);

[[nodiscard]] bool isNearlyZero(
    Scalar value,
    Scalar tolerance = defaultAbsoluteTolerance);

}
