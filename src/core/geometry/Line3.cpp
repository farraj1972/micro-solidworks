#include "core/geometry/Line3.h"
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
        throw std::invalid_argument{"Line3 direction must be finite"};

    const auto scale = std::max({std::abs(direction.x()), std::abs(direction.y()), std::abs(direction.z())});
    // A component larger than the threshold already proves non-degeneracy.
    // Otherwise hypot is safe and retains the exact geometric boundary policy.
    if (scale <= defaultGeometricTolerance
        && std::hypot(direction.x(), direction.y(), direction.z()) <= defaultGeometricTolerance)
        throw std::invalid_argument{"Line3 direction is geometrically near zero"};

    // Scale before computing the norm: even finite inputs whose full magnitude
    // is unrepresentable have a finite, orientation-preserving unit direction.
    const auto x = direction.x() / scale;
    const auto y = direction.y() / scale;
    const auto z = direction.z() / scale;
    const auto magnitude = std::hypot(x, y, z);
    return {x / magnitude, y / magnitude, z / magnitude};
}

}

Line3::Line3(const Point3& origin, const math::Vector3& direction)
    : origin_{origin}, direction_{validatedUnitDirection(direction)}
{
}

}

namespace microsw::geometry
{
Point3 Line3::pointAt(math::Scalar t) const
{
    detail::validateParameter(t);
    if (t == 0) return origin_;
    return origin_ + t * direction_;
}

bool Line3::contains(const Point3& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return detail::onSupport(origin_, direction_, point, tolerance);
}

bool isParallel(const Line3& a, const Line3& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::parallel(first, second, tolerance);
}

bool isPerpendicular(const Line3& a, const Line3& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::perpendicular(first, second, tolerance);
}
}
