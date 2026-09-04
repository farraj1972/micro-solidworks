#pragma once

#include "core/geometry/Point2.h"

namespace microsw::geometry
{

// Ordered endpoints only; coincident endpoints are valid geometric values.
class Segment2
{
public:
    constexpr Segment2() noexcept = default;
    constexpr Segment2(const Point2& a, const Point2& b) noexcept : a_{a}, b_{b} {}

    [[nodiscard]] constexpr const Point2& a() const noexcept { return a_; }
    [[nodiscard]] constexpr const Point2& b() const noexcept { return b_; }

    // Non-finite length or squared length throws std::overflow_error.
    [[nodiscard]] math::Scalar length() const;
    [[nodiscard]] math::Scalar squaredLength() const;
    [[nodiscard]] Point2 midpoint() const;
    // Uses the default geometric tolerance; degeneracy throws std::domain_error.
    // Scaling preserves direction even when the full displacement overflows.
    [[nodiscard]] math::Vector2 direction() const;
    // Same finite, non-negative tolerance policy as areCoincident; zero is exact.
    [[nodiscard]] bool isDegenerate(
        math::Scalar tolerance = defaultGeometricTolerance) const;

    // Finite parameter required; outside the primitive domain throws domain_error.
    // Non-representable results throw overflow_error.
    [[nodiscard]] Point2 pointAt(math::Scalar t) const;
    // Finite non-negative tolerance; zero uses the exact computed residual.
    [[nodiscard]] bool contains(const Point2& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    Point2 a_{};
    Point2 b_{};
};

// Relations use dimensionless unit-vector residuals and the geometric default.
[[nodiscard]] bool isParallel(const Segment2& a, const Segment2& b,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] bool isPerpendicular(const Segment2& a, const Segment2& b,
    math::Scalar tolerance = defaultGeometricTolerance);

}
