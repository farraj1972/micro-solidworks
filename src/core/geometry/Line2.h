#pragma once

#include "core/geometry/Point2.h"

namespace microsw::geometry
{

// Infinite oriented line: origin + t * direction, t in R.
// Only the supplied origin and a unit direction are stored.
class Line2
{
public:
    // Non-finite directions or Euclidean magnitude <= the default geometric
    // tolerance throw std::invalid_argument. Normalization preserves orientation.
    Line2(const Point2& origin, const math::Vector2& direction);

    [[nodiscard]] const Point2& origin() const noexcept { return origin_; }
    [[nodiscard]] const math::Vector2& direction() const noexcept { return direction_; }

    // Finite parameter required; outside the primitive domain throws domain_error.
    // Non-representable results throw overflow_error.
    [[nodiscard]] Point2 pointAt(math::Scalar t) const;
    // Finite non-negative tolerance; zero uses the exact computed residual.
    [[nodiscard]] bool contains(const Point2& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    Point2 origin_;
    math::Vector2 direction_;
};

// Metric operations do not snap results using geometric tolerance.
// Non-representable results throw std::overflow_error.
[[nodiscard]] Point2 closestPoint(const Line2& primitive, const Point2& point);
[[nodiscard]] math::Scalar distance(const Line2& primitive, const Point2& point);

// Relations use dimensionless unit-vector residuals and the geometric default.
[[nodiscard]] bool isParallel(const Line2& a, const Line2& b,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] bool isPerpendicular(const Line2& a, const Line2& b,
    math::Scalar tolerance = defaultGeometricTolerance);

}
