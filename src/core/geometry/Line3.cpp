#include "core/geometry/Line3.h"

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
