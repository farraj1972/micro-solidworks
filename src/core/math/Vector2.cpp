#include "core/math/Vector2.h"

#include <cmath>
#include <stdexcept>

namespace microsw::math
{

Scalar Vector2::length() const
{
    return std::sqrt(squaredLength());
}

Vector2 Vector2::normalized() const
{
    const Scalar magnitude = length();
    if (isNearlyZero(magnitude))
    {
        throw std::domain_error{"cannot normalize a zero-length vector"};
    }

    return *this / magnitude;
}

Vector2 operator/(const Vector2& vector, Scalar scalar)
{
    if (isNearlyZero(scalar))
    {
        throw std::domain_error{"cannot divide a vector by an effectively zero scalar"};
    }

    return {vector.x() / scalar, vector.y() / scalar};
}

bool almostEqual(
    const Vector2& first,
    const Vector2& second,
    Scalar absoluteTolerance,
    Scalar relativeTolerance)
{
    return almostEqual(first.x(), second.x(), absoluteTolerance, relativeTolerance)
        && almostEqual(first.y(), second.y(), absoluteTolerance, relativeTolerance);
}

}
