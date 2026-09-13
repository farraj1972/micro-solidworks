#include "core/geometry/Circle2.h"
#include "core/geometry/GeometryQuerySupport.h"

#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{
math::Scalar checkedCoordinate(math::Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"Circular point is not representable"};
    return value;
}
}

Circle2::Circle2(const Point2& center, math::Scalar radius)
    : center_{center}, radius_{radius}
{
    if (!std::isfinite(radius) || radius <= defaultGeometricTolerance)
        throw std::invalid_argument{"Circle2 radius must be finite and greater than geometric tolerance"};
}

Point2 Circle2::pointAt(math::Scalar angle) const
{
    detail::validateParameter(angle);
    return {checkedCoordinate(center_.x() + radius_ * std::cos(angle)),
            checkedCoordinate(center_.y() + radius_ * std::sin(angle))};
}

Point2 Circle2::closestPoint(const Point2& point) const
{
    const auto dx = point.x() - center_.x();
    const auto dy = point.y() - center_.y();
    const auto magnitude = std::hypot(dx, dy);
    if (magnitude == 0) return pointAt(0);
    if (!std::isfinite(magnitude))
        throw std::overflow_error{"Circle2 query displacement is not representable"};
    return {checkedCoordinate(center_.x() + radius_ * dx / magnitude),
            checkedCoordinate(center_.y() + radius_ * dy / magnitude)};
}

math::Scalar Circle2::distance(const Point2& point) const
{
    const auto radialDistance = std::hypot(point.x() - center_.x(), point.y() - center_.y());
    const auto result = std::abs(radialDistance - radius_);
    if (!std::isfinite(result))
        throw std::overflow_error{"Circle2 distance is not representable"};
    return result;
}

bool Circle2::contains(const Point2& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return distance(point) <= tolerance;
}

}
