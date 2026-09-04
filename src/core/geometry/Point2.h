#pragma once

#include "core/geometry/GeometricTolerance.h"
#include "core/math/Vector2.h"

namespace microsw::geometry
{

// Finite geometric position, distinct from a displacement and from CAD identity.
// Value semantics; coordinates are read-only and no Point/Vector conversion exists.
class Point2
{
public:
    constexpr Point2() noexcept = default;
    // Non-finite coordinates throw std::invalid_argument.
    Point2(math::Scalar x, math::Scalar y);

    [[nodiscard]] constexpr math::Scalar x() const noexcept { return x_; }
    [[nodiscard]] constexpr math::Scalar y() const noexcept { return y_; }

private:
    math::Scalar x_{};
    math::Scalar y_{};
};

// Non-finite vector inputs throw std::invalid_argument; non-finite arithmetic
// results (including a displacement) throw std::overflow_error.
[[nodiscard]] math::Vector2 operator-(const Point2& first, const Point2& second);
[[nodiscard]] Point2 operator+(const Point2& point, const math::Vector2& vector);
[[nodiscard]] Point2 operator+(const math::Vector2& vector, const Point2& point);
[[nodiscard]] Point2 operator-(const Point2& point, const math::Vector2& vector);

// Euclidean distance <= tolerance, with no relative tolerance. Tolerance must
// be finite and >= 0 (invalid_argument otherwise); zero means exact position.
// Large finite coordinates are compared without squaring overflow.
[[nodiscard]] bool areCoincident(
    const Point2& first, const Point2& second,
    math::Scalar tolerance = defaultGeometricTolerance);

}
