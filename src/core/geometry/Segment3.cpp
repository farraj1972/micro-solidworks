#include "core/geometry/Segment3.h"
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
        throw std::overflow_error{"Segment3 length result is not finite"};
    return value;
}

}

math::Scalar Segment3::length() const
{
    const auto displacement = b_ - a_;
    return checkedResult(std::hypot(displacement.x(), displacement.y(), displacement.z()));
}

math::Scalar Segment3::squaredLength() const
{
    const auto magnitude = length();
    return checkedResult(magnitude * magnitude);
}

Point3 Segment3::midpoint() const
{
    // Coordinate-wise affine midpoint avoids both A+B and B-A overflow.
    // For finite endpoints it lies between them and is always representable.
    return {std::midpoint(a_.x(), b_.x()), std::midpoint(a_.y(), b_.y()), std::midpoint(a_.z(), b_.z())};
}

bool Segment3::isDegenerate(math::Scalar tolerance) const
{
    return areCoincident(a_, b_, tolerance);
}

math::Vector3 Segment3::direction() const
{
    if (isDegenerate())
        throw std::domain_error{"A degenerate Segment3 has no direction"};

    auto dx = b_.x() - a_.x();
    auto dy = b_.y() - a_.y();
    auto dz = b_.z() - a_.z();
    if (!std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz))
    {
        // Only an overflowing difference needs endpoint halving. Apply the
        // same factor to every component to preserve the displacement ratio.
        dx = b_.x() * 0.5 - a_.x() * 0.5;
        dy = b_.y() * 0.5 - a_.y() * 0.5;
        dz = b_.z() * 0.5 - a_.z() * 0.5;
    }
    // Normalize scaled components: neither squaring nor the norm can overflow.
    const auto scale = std::max({std::abs(dx), std::abs(dy), std::abs(dz)});
    dx /= scale;
    dy /= scale;
    dz /= scale;
    const auto magnitude = std::hypot(dx, dy, dz);
    return {dx / magnitude, dy / magnitude, dz / magnitude};
}

}

namespace microsw::geometry
{
Point3 Segment3::pointAt(math::Scalar t) const
{
    detail::validateParameter(t);
    if (t < 0 || t > 1) throw std::domain_error{"Segment parameter must be in [0,1]"};
    if (t == 0) return a_;
    if (t == 1) return b_;
    // lerp avoids an overflowing B-A while preserving finite convex interpolation.
    return {checkedResult(std::lerp(a_.x(), b_.x(), t)), checkedResult(std::lerp(a_.y(), b_.y(), t)), checkedResult(std::lerp(a_.z(), b_.z(), t))};
}

bool Segment3::contains(const Point3& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return detail::inSegment(a_, b_, point, tolerance);
}

bool isParallel(const Segment3& a, const Segment3& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::parallel(first, second, tolerance);
}

bool isPerpendicular(const Segment3& a, const Segment3& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::perpendicular(first, second, tolerance);
}

Point3 closestPoint(const Segment3& primitive, const Point3& point)
{
    return detail::nearestSegment(primitive.a(), primitive.b(), point);
}

math::Scalar distance(const Segment3& primitive, const Point3& point)
{
    const auto nearest = closestPoint(primitive, point);
    return detail::pointMetric(point, nearest);
}
}
