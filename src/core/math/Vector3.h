#pragma once

#include "core/math/Scalar.h"
#include "core/math/Tolerance.h"

namespace microsw::math
{

class Vector3
{
public:
    constexpr Vector3() = default;
    constexpr Vector3(Scalar x, Scalar y, Scalar z) noexcept
        : x_{x}, y_{y}, z_{z}
    {
    }

    [[nodiscard]] constexpr Scalar x() const noexcept { return x_; }
    [[nodiscard]] constexpr Scalar y() const noexcept { return y_; }
    [[nodiscard]] constexpr Scalar z() const noexcept { return z_; }

    [[nodiscard]] constexpr Vector3 operator-() const noexcept
    {
        return {-x_, -y_, -z_};
    }

    [[nodiscard]] constexpr Scalar squaredLength() const noexcept
    {
        return x_ * x_ + y_ * y_ + z_ * z_;
    }

    [[nodiscard]] Scalar length() const;
    [[nodiscard]] Vector3 normalized() const;

private:
    Scalar x_{};
    Scalar y_{};
    Scalar z_{};
};

[[nodiscard]] constexpr Vector3 operator+(const Vector3& first, const Vector3& second) noexcept
{
    return {first.x() + second.x(), first.y() + second.y(), first.z() + second.z()};
}

[[nodiscard]] constexpr Vector3 operator-(const Vector3& first, const Vector3& second) noexcept
{
    return {first.x() - second.x(), first.y() - second.y(), first.z() - second.z()};
}

[[nodiscard]] constexpr Vector3 operator*(const Vector3& vector, Scalar scalar) noexcept
{
    return {vector.x() * scalar, vector.y() * scalar, vector.z() * scalar};
}

[[nodiscard]] constexpr Vector3 operator*(Scalar scalar, const Vector3& vector) noexcept
{
    return vector * scalar;
}

[[nodiscard]] Vector3 operator/(const Vector3& vector, Scalar scalar);

[[nodiscard]] constexpr Scalar dot(const Vector3& first, const Vector3& second) noexcept
{
    return first.x() * second.x() + first.y() * second.y() + first.z() * second.z();
}

[[nodiscard]] constexpr Vector3 cross(const Vector3& first, const Vector3& second) noexcept
{
    return {
        first.y() * second.z() - first.z() * second.y(),
        first.z() * second.x() - first.x() * second.z(),
        first.x() * second.y() - first.y() * second.x()};
}

[[nodiscard]] bool almostEqual(
    const Vector3& first,
    const Vector3& second,
    Scalar absoluteTolerance = defaultAbsoluteTolerance,
    Scalar relativeTolerance = defaultRelativeTolerance);

}
