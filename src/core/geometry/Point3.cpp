#include "core/geometry/Point3.h"

#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Scalar checkedResult(math::Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"Point3 arithmetic result is not finite"};
    return value;
}

void validateVector(const math::Vector3& vector)
{
    if (!std::isfinite(vector.x()) || !std::isfinite(vector.y()) || !std::isfinite(vector.z()))
        throw std::invalid_argument{"Point3 displacement must be finite"};
}

}

Point3::Point3(math::Scalar x, math::Scalar y, math::Scalar z)
    : x_{x}, y_{y}, z_{z}
{
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        throw std::invalid_argument{"Point3 coordinates must be finite"};
}

math::Vector3 operator-(const Point3& first, const Point3& second)
{
    return {checkedResult(first.x() - second.x()),
            checkedResult(first.y() - second.y()),
            checkedResult(first.z() - second.z())};
}

Point3 operator+(const Point3& point, const math::Vector3& vector)
{
    validateVector(vector);
    return {checkedResult(point.x() + vector.x()),
            checkedResult(point.y() + vector.y()),
            checkedResult(point.z() + vector.z())};
}

Point3 operator+(const math::Vector3& vector, const Point3& point)
{
    return point + vector;
}

Point3 operator-(const Point3& point, const math::Vector3& vector)
{
    validateVector(vector);
    return {checkedResult(point.x() - vector.x()),
            checkedResult(point.y() - vector.y()),
            checkedResult(point.z() - vector.z())};
}

bool areCoincident(const Point3& first, const Point3& second, math::Scalar tolerance)
{
    if (!std::isfinite(tolerance) || tolerance < 0.0)
        throw std::invalid_argument{"Geometric tolerance must be finite and non-negative"};
    if (tolerance == 0.0)
        return first.x() == second.x() && first.y() == second.y() && first.z() == second.z();

    const auto dx = first.x() - second.x();
    const auto dy = first.y() - second.y();
    const auto dz = first.z() - second.z();
    // Finite endpoints can have an unrepresentable difference. Its magnitude
    // exceeds every valid finite tolerance, so coincidence is false, not an error.
    if (!std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz))
        return false;
    // hypot avoids intermediate square overflow/underflow. An unrepresentable
    // norm is also greater than every finite tolerance.
    return std::hypot(dx, dy, dz) <= tolerance;
}

}
