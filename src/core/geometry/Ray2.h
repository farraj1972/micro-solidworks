#pragma once

#include "core/geometry/Point2.h"

namespace microsw::geometry
{

// Oriented half-line: origin + t * direction, t >= 0.
// Reversing direction selects the opposite half-line; no sign canonicalization.
// Only the supplied origin and a unit direction are stored.
class Ray2
{
public:
    // Non-finite directions or Euclidean magnitude <= the default geometric
    // tolerance throw std::invalid_argument. Normalization preserves orientation.
    Ray2(const Point2& origin, const math::Vector2& direction);

    [[nodiscard]] const Point2& origin() const noexcept { return origin_; }
    [[nodiscard]] const math::Vector2& direction() const noexcept { return direction_; }

private:
    Point2 origin_;
    math::Vector2 direction_;
};

}
