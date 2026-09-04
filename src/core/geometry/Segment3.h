#pragma once

#include "core/geometry/Point3.h"

namespace microsw::geometry
{

// Ordered endpoints only; coincident endpoints are valid geometric values.
class Segment3
{
public:
    constexpr Segment3() noexcept = default;
    constexpr Segment3(const Point3& a, const Point3& b) noexcept : a_{a}, b_{b} {}

    [[nodiscard]] constexpr const Point3& a() const noexcept { return a_; }
    [[nodiscard]] constexpr const Point3& b() const noexcept { return b_; }

    // Non-finite length or squared length throws std::overflow_error.
    [[nodiscard]] math::Scalar length() const;
    [[nodiscard]] math::Scalar squaredLength() const;
    [[nodiscard]] Point3 midpoint() const;
    // Uses the default geometric tolerance; degeneracy throws std::domain_error.
    // Scaling preserves direction even when the full displacement overflows.
    [[nodiscard]] math::Vector3 direction() const;
    // Same finite, non-negative tolerance policy as areCoincident; zero is exact.
    [[nodiscard]] bool isDegenerate(
        math::Scalar tolerance = defaultGeometricTolerance) const;

    // Finite parameter required; outside the primitive domain throws domain_error.
    // Non-representable results throw overflow_error.
    [[nodiscard]] Point3 pointAt(math::Scalar t) const;
    // Finite non-negative tolerance; zero uses the exact computed residual.
    [[nodiscard]] bool contains(const Point3& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    Point3 a_{};
    Point3 b_{};
};

// Metric operations do not snap results using geometric tolerance.
// Non-representable results throw std::overflow_error.
[[nodiscard]] Point3 closestPoint(const Segment3& primitive, const Point3& point);
[[nodiscard]] math::Scalar distance(const Segment3& primitive, const Point3& point);

// Relations use dimensionless unit-vector residuals and the geometric default.
[[nodiscard]] bool isParallel(const Segment3& a, const Segment3& b,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] bool isPerpendicular(const Segment3& a, const Segment3& b,
    math::Scalar tolerance = defaultGeometricTolerance);

}
