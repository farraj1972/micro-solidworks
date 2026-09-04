#pragma once

#include "core/geometry/Point3.h"

namespace microsw::geometry
{

// Oriented half-line: origin + t * direction, t >= 0.
// Reversing direction selects the opposite half-line; no sign canonicalization.
// Only the supplied origin and a unit direction are stored.
class Ray3
{
public:
    // Non-finite directions or Euclidean magnitude <= the default geometric
    // tolerance throw std::invalid_argument. Normalization preserves orientation.
    Ray3(const Point3& origin, const math::Vector3& direction);

    [[nodiscard]] const Point3& origin() const noexcept { return origin_; }
    [[nodiscard]] const math::Vector3& direction() const noexcept { return direction_; }

private:
    Point3 origin_;
    math::Vector3 direction_;
};

}
