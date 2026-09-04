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

private:
    Point3 a_{};
    Point3 b_{};
};

}
