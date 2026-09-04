#include "core/geometry/Point2.h"

#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Scalar checkedResult(math::Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"Point2 arithmetic result is not finite"};
    return value;
}

void validateVector(const math::Vector2& vector)
{
    if (!std::isfinite(vector.x()) || !std::isfinite(vector.y()))
        throw std::invalid_argument{"Point2 displacement must be finite"};
}

}

Point2::Point2(math::Scalar x, math::Scalar y)
    : x_{x}, y_{y}
{
    if (!std::isfinite(x) || !std::isfinite(y))
        throw std::invalid_argument{"Point2 coordinates must be finite"};
}

math::Vector2 operator-(const Point2& first, const Point2& second)
{
    return {checkedResult(first.x() - second.x()),
            checkedResult(first.y() - second.y())};
}

Point2 operator+(const Point2& point, const math::Vector2& vector)
{
    validateVector(vector);
    return {checkedResult(point.x() + vector.x()),
            checkedResult(point.y() + vector.y())};
}

Point2 operator+(const math::Vector2& vector, const Point2& point)
{
    return point + vector;
}

Point2 operator-(const Point2& point, const math::Vector2& vector)
{
    validateVector(vector);
    return {checkedResult(point.x() - vector.x()),
            checkedResult(point.y() - vector.y())};
}

bool areCoincident(const Point2& first, const Point2& second, math::Scalar tolerance)
{
    if (!std::isfinite(tolerance) || tolerance < 0.0)
        throw std::invalid_argument{"Geometric tolerance must be finite and non-negative"};
    if (tolerance == 0.0)
        return first.x() == second.x() && first.y() == second.y();

    const auto dx = first.x() - second.x();
    const auto dy = first.y() - second.y();
    // Finite endpoints can have an unrepresentable difference. Its magnitude
    // exceeds every valid finite tolerance, so coincidence is false, not an error.
    if (!std::isfinite(dx) || !std::isfinite(dy))
        return false;
    // hypot avoids intermediate square overflow/underflow. An unrepresentable
    // norm is also greater than every finite tolerance.
    return std::hypot(dx, dy) <= tolerance;
}

}
