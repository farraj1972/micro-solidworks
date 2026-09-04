#pragma once

#include "core/geometry/GeometricTolerance.h"
#include "core/math/Vector3.h"

namespace microsw::geometry
{

// Finite geometric position, distinct from a displacement and from CAD identity.
// Value semantics; coordinates are read-only and no Point/Vector conversion exists.
class Point3
{
public:
    constexpr Point3() noexcept = default;
    // Non-finite coordinates throw std::invalid_argument.
    Point3(math::Scalar x, math::Scalar y, math::Scalar z);

    [[nodiscard]] constexpr math::Scalar x() const noexcept { return x_; }
    [[nodiscard]] constexpr math::Scalar y() const noexcept { return y_; }
    [[nodiscard]] constexpr math::Scalar z() const noexcept { return z_; }

private:
    math::Scalar x_{};
    math::Scalar y_{};
    math::Scalar z_{};
};

// Non-finite vector inputs throw std::invalid_argument; non-finite arithmetic
// results (including a displacement) throw std::overflow_error.
[[nodiscard]] math::Vector3 operator-(const Point3& first, const Point3& second);
[[nodiscard]] Point3 operator+(const Point3& point, const math::Vector3& vector);
[[nodiscard]] Point3 operator+(const math::Vector3& vector, const Point3& point);
[[nodiscard]] Point3 operator-(const Point3& point, const math::Vector3& vector);

// Euclidean distance <= tolerance, with no relative tolerance. Tolerance must
// be finite and >= 0 (invalid_argument otherwise); zero means exact position.
// Large finite coordinates are compared without squaring overflow.
[[nodiscard]] bool areCoincident(
    const Point3& first, const Point3& second,
    math::Scalar tolerance = defaultGeometricTolerance);

}
