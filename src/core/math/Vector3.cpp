#include "core/math/Vector3.h"

#include <cmath>
#include <stdexcept>

namespace microsw::math
{

Scalar Vector3::length() const
{
    return std::sqrt(squaredLength());
}

Vector3 Vector3::normalized() const
{
    const Scalar magnitude = length();
    if (isNearlyZero(magnitude))
    {
        throw std::domain_error{"cannot normalize a zero-length vector"};
    }
    return *this / magnitude;
}

Vector3 operator/(const Vector3& vector, Scalar scalar)
{
    if (isNearlyZero(scalar))
    {
        throw std::domain_error{"cannot divide a vector by an effectively zero scalar"};
    }
    return {vector.x() / scalar, vector.y() / scalar, vector.z() / scalar};
}

bool almostEqual(
    const Vector3& first,
    const Vector3& second,
    Scalar absoluteTolerance,
    Scalar relativeTolerance)
{
    return almostEqual(first.x(), second.x(), absoluteTolerance, relativeTolerance)
        && almostEqual(first.y(), second.y(), absoluteTolerance, relativeTolerance)
        && almostEqual(first.z(), second.z(), absoluteTolerance, relativeTolerance);
}

}
