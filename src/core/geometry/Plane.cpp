#include "core/geometry/Plane.h"
#include "core/geometry/GeometryQuerySupport.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Vector3 validatedUnitNormal(const math::Vector3& normal)
{
    if (!std::isfinite(normal.x()) || !std::isfinite(normal.y()) || !std::isfinite(normal.z()))
        throw std::invalid_argument{"Plane normal must be finite"};

    const auto scale = std::max({std::abs(normal.x()), std::abs(normal.y()), std::abs(normal.z())});
    // A component larger than the threshold already proves non-degeneracy.
    // Otherwise hypot is safe and retains the exact geometric boundary policy.
    if (scale <= defaultGeometricTolerance
        && std::hypot(normal.x(), normal.y(), normal.z()) <= defaultGeometricTolerance)
        throw std::invalid_argument{"Plane normal is geometrically near zero"};

    // Scale before computing the norm: even finite inputs whose full magnitude
    // is unrepresentable have a finite, orientation-preserving unit normal.
    const auto x = normal.x() / scale;
    const auto y = normal.y() / scale;
    const auto z = normal.z() / scale;
    const auto magnitude = std::hypot(x, y, z);
    return {x / magnitude, y / magnitude, z / magnitude};
}

}

Plane::Plane(const Point3& origin, const math::Vector3& normal)
    : origin_{origin}, normal_{validatedUnitNormal(normal)}
{
}

}

namespace microsw::geometry
{
bool Plane::contains(const Point3& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return detail::onPlane(origin_, normal_, point, tolerance);
}

bool isParallel(const Plane& a, const Plane& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.normal();
    const auto second = b.normal();
    return detail::parallel(first, second, tolerance);
}

bool isPerpendicular(const Plane& a, const Plane& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.normal();
    const auto second = b.normal();
    return detail::perpendicular(first, second, tolerance);
}
}
