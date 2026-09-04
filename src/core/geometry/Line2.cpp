#include "core/geometry/Line2.h"
#include "core/geometry/GeometryQuerySupport.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Vector2 validatedUnitDirection(const math::Vector2& direction)
{
    if (!std::isfinite(direction.x()) || !std::isfinite(direction.y()))
        throw std::invalid_argument{"Line2 direction must be finite"};

    const auto scale = std::max({std::abs(direction.x()), std::abs(direction.y())});
    // A component larger than the threshold already proves non-degeneracy.
    // Otherwise hypot is safe and retains the exact geometric boundary policy.
    if (scale <= defaultGeometricTolerance
        && std::hypot(direction.x(), direction.y()) <= defaultGeometricTolerance)
        throw std::invalid_argument{"Line2 direction is geometrically near zero"};

    // Scale before computing the norm: even finite inputs whose full magnitude
    // is unrepresentable have a finite, orientation-preserving unit direction.
    const auto x = direction.x() / scale;
    const auto y = direction.y() / scale;
    const auto magnitude = std::hypot(x, y);
    return {x / magnitude, y / magnitude};
}

}

Line2::Line2(const Point2& origin, const math::Vector2& direction)
    : origin_{origin}, direction_{validatedUnitDirection(direction)}
{
}

}

namespace microsw::geometry
{
Point2 Line2::pointAt(math::Scalar t) const
{
    detail::validateParameter(t);
    if (t == 0) return origin_;
    return origin_ + t * direction_;
}

bool Line2::contains(const Point2& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return detail::onSupport(Point3{origin_.x(), origin_.y(), 0}, math::Vector3{direction_.x(), direction_.y(), 0}, Point3{point.x(), point.y(), 0}, tolerance);
}

bool isParallel(const Line2& a, const Line2& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::parallel(math::Vector3{first.x(), first.y(), 0}, math::Vector3{second.x(), second.y(), 0}, tolerance);
}

bool isPerpendicular(const Line2& a, const Line2& b, math::Scalar tolerance)
{
    detail::validateTolerance(tolerance);
    const auto first = a.direction();
    const auto second = b.direction();
    return detail::perpendicular(math::Vector3{first.x(), first.y(), 0}, math::Vector3{second.x(), second.y(), 0}, tolerance);
}
}
