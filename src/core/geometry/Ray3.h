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

    // Finite parameter required; outside the primitive domain throws domain_error.
    // Non-representable results throw overflow_error.
    [[nodiscard]] Point3 pointAt(math::Scalar t) const;
    // Finite non-negative tolerance; zero uses the exact computed residual.
    [[nodiscard]] bool contains(const Point3& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    Point3 origin_;
    math::Vector3 direction_;
};

// Relations use dimensionless unit-vector residuals and the geometric default.
[[nodiscard]] bool isParallel(const Ray3& a, const Ray3& b,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] bool isPerpendicular(const Ray3& a, const Ray3& b,
    math::Scalar tolerance = defaultGeometricTolerance);

}
