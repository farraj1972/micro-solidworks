#pragma once

#include "core/math/Scalar.h"
#include "core/math/Tolerance.h"

namespace microsw::math
{

class Vector2
{
public:
    constexpr Vector2() = default;
    constexpr Vector2(Scalar x, Scalar y) noexcept
        : x_{x}, y_{y}
    {
    }

    [[nodiscard]] constexpr Scalar x() const noexcept
    {
        return x_;
    }

    [[nodiscard]] constexpr Scalar y() const noexcept
    {
        return y_;
    }

    [[nodiscard]] constexpr Vector2 operator-() const noexcept
    {
        return {-x_, -y_};
    }

    [[nodiscard]] constexpr Scalar squaredLength() const noexcept
    {
        return x_ * x_ + y_ * y_;
    }

    [[nodiscard]] Scalar length() const;
    [[nodiscard]] Vector2 normalized() const;

private:
    Scalar x_{};
    Scalar y_{};
};

[[nodiscard]] constexpr Vector2 operator+(const Vector2& first, const Vector2& second) noexcept
{
    return {first.x() + second.x(), first.y() + second.y()};
}

[[nodiscard]] constexpr Vector2 operator-(const Vector2& first, const Vector2& second) noexcept
{
    return {first.x() - second.x(), first.y() - second.y()};
}

[[nodiscard]] constexpr Vector2 operator*(const Vector2& vector, Scalar scalar) noexcept
{
    return {vector.x() * scalar, vector.y() * scalar};
}

[[nodiscard]] constexpr Vector2 operator*(Scalar scalar, const Vector2& vector) noexcept
{
    return vector * scalar;
}

[[nodiscard]] Vector2 operator/(const Vector2& vector, Scalar scalar);

[[nodiscard]] constexpr Scalar dot(const Vector2& first, const Vector2& second) noexcept
{
    return first.x() * second.x() + first.y() * second.y();
}

[[nodiscard]] bool almostEqual(
    const Vector2& first,
    const Vector2& second,
    Scalar absoluteTolerance = defaultAbsoluteTolerance,
    Scalar relativeTolerance = defaultRelativeTolerance);

}
