#include "core/geometry/Line2.h"

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
