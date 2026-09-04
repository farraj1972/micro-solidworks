#include "core/geometry/Segment2.h"
#include "core/geometry/GeometryQuerySupport.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Scalar checkedResult(math::Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"Segment2 length result is not finite"};
    return value;
}

}

math::Scalar Segment2::length() const
{
    const auto displacement = b_ - a_;
    return checkedResult(std::hypot(displacement.x(), displacement.y()));
}

math::Scalar Segment2::squaredLength() const
{
    const auto magnitude = length();
    return checkedResult(magnitude * magnitude);
}

Point2 Segment2::midpoint() const
{
    // Coordinate-wise affine midpoint avoids both A+B and B-A overflow.
    // For finite endpoints it lies between them and is always representable.
    return {std::midpoint(a_.x(), b_.x()), std::midpoint(a_.y(), b_.y())};
}

bool Segment2::isDegenerate(math::Scalar tolerance) const
{
    return areCoincident(a_, b_, tolerance);
}

math::Vector2 Segment2::direction() const
{
    if (isDegenerate())
        throw std::domain_error{"A degenerate Segment2 has no direction"};

    auto dx = b_.x() - a_.x();
    auto dy = b_.y() - a_.y();
    if (!std::isfinite(dx) || !std::isfinite(dy))
    {
        // Only an overflowing difference needs endpoint halving. Apply the
        // same factor to every component to preserve the displacement ratio.
        dx = b_.x() * 0.5 - a_.x() * 0.5;
        dy = b_.y() * 0.5 - a_.y() * 0.5;
    }
    // Normalize scaled components: neither squaring nor the norm can overflow.
    const auto scale = std::max({std::abs(dx), std::abs(dy)});
    dx /= scale;
    dy /= scale;
    const auto magnitude = std::hypot(dx, dy);
    return {dx / magnitude, dy / magnitude};
}

}

namespace microsw::geometry
{
Point2 Segment2::pointAt(math::Scalar t) const
{
    detail::validateParameter(t);
    if (t < 0 || t > 1) throw std::domain_error{"Segment parameter must be in [0,1]"};
    if (t == 0) return a_;
    if (t == 1) return b_;
    // lerp avoids an overflowing B-A while preserving finite convex interpolation.
    return {checkedResult(std::lerp(a_.x(), b_.x(), t)), checkedResult(std::lerp(a_.y(), b_.y(), t))};
}

bool Segment2::contains(const Point2& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return detail::inSegment(Point3{a_.x(), a_.y(), 0}, Point3{b_.x(), b_.y(), 0}, Point3{point.x(), point.y(), 0}, tolerance);
}

bool isParallel(const Segment2& a, const Segment2& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::parallel(math::Vector3{first.x(), first.y(), 0}, math::Vector3{second.x(), second.y(), 0}, tolerance);
}

bool isPerpendicular(const Segment2& a, const Segment2& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::perpendicular(math::Vector3{first.x(), first.y(), 0}, math::Vector3{second.x(), second.y(), 0}, tolerance);
}

Point2 closestPoint(const Segment2& primitive, const Point2& point)
{
    const auto result = detail::nearestSegment(Point3{primitive.a().x(), primitive.a().y(), 0}, Point3{primitive.b().x(), primitive.b().y(), 0}, Point3{point.x(), point.y(), 0});
    return {result.x(), result.y()};
}

math::Scalar distance(const Segment2& primitive, const Point2& point)
{
    const auto nearest = closestPoint(primitive, point);
    return detail::pointMetric(Point3{point.x(), point.y(), 0}, Point3{nearest.x(), nearest.y(), 0});
}
}
