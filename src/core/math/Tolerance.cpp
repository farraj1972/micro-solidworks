#include "core/math/Tolerance.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace microsw::math
{
namespace
{

void validateTolerance(Scalar tolerance, const char* name)
{
    if (tolerance < 0.0)
    {
        throw std::invalid_argument(name);
    }
}

}

bool almostEqual(
    Scalar first,
    Scalar second,
    Scalar absoluteTolerance,
    Scalar relativeTolerance)
{
    validateTolerance(absoluteTolerance, "absolute tolerance must be non-negative");
    validateTolerance(relativeTolerance, "relative tolerance must be non-negative");

    if (first == second)
    {
        return true;
    }

    if (!std::isfinite(first) || !std::isfinite(second))
    {
        return false;
    }

    const Scalar difference = std::abs(first - second);
    const Scalar scale = std::max(std::abs(first), std::abs(second));
    const Scalar threshold =
        std::max(absoluteTolerance, relativeTolerance * scale);

    return difference <= threshold;
}

bool isNearlyZero(Scalar value, Scalar tolerance)
{
    validateTolerance(tolerance, "tolerance must be non-negative");
    return std::isfinite(value) && std::abs(value) <= tolerance;
}

}
