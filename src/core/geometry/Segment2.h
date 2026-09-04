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

private:
    Point2 a_{};
    Point2 b_{};
};

}
