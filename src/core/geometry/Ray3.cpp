#include "core/geometry/Ray3.h"
#include "core/geometry/GeometryQuerySupport.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Vector3 validatedUnitDirection(const math::Vector3& direction)
{
    if (!std::isfinite(direction.x()) || !std::isfinite(direction.y()) || !std::isfinite(direction.z()))
        throw std::invalid_argument{"Ray3 direction must be finite"};

    const auto scale = std::max({std::abs(direction.x()), std::abs(direction.y()), std::abs(direction.z())});
    // A component larger than the threshold already proves non-degeneracy.
    // Otherwise hypot is safe and retains the exact geometric boundary policy.
    if (scale <= defaultGeometricTolerance
        && std::hypot(direction.x(), direction.y(), direction.z()) <= defaultGeometricTolerance)
        throw std::invalid_argument{"Ray3 direction is geometrically near zero"};

    // Scale before computing the norm: even finite inputs whose full magnitude
    // is unrepresentable have a finite, orientation-preserving unit direction.
    const auto x = direction.x() / scale;
    const auto y = direction.y() / scale;
    const auto z = direction.z() / scale;
    const auto magnitude = std::hypot(x, y, z);
    return {x / magnitude, y / magnitude, z / magnitude};
}

}

Ray3::Ray3(const Point3& origin, const math::Vector3& direction)
    : origin_{origin}, direction_{validatedUnitDirection(direction)}
{
}

}

namespace microsw::geometry
{
Point3 Ray3::pointAt(math::Scalar t) const
{
    detail::validateParameter(t);
    if (t < 0) throw std::domain_error{"Ray parameter must be non-negative"};
    if (t == 0) return origin_;
    return origin_ + t * direction_;
}

bool Ray3::contains(const Point3& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return detail::onSupport(origin_, direction_, point, tolerance)
        && detail::inForwardHalfSpace(origin_, direction_, point, tolerance);
}

bool isParallel(const Ray3& a, const Ray3& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::parallel(first, second, tolerance);
}

bool isPerpendicular(const Ray3& a, const Ray3& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::perpendicular(first, second, tolerance);
}

Point3 closestPoint(const Ray3& primitive, const Point3& point)
{
    return detail::nearestLine(primitive.origin(), primitive.direction(), point, true);
}

math::Scalar distance(const Ray3& primitive, const Point3& point)
{
    return detail::lineMetric(primitive.origin(), primitive.direction(), point, true);
}
}
