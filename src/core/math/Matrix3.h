#pragma once

#include "core/math/Scalar.h"
#include "core/math/Tolerance.h"
#include "core/math/Vector3.h"

#include <array>
#include <cstddef>
#include <stdexcept>

namespace microsw::math
{

class Matrix3
{
public:
    constexpr Matrix3() = default;

    constexpr Matrix3(
        Scalar m00, Scalar m01, Scalar m02,
        Scalar m10, Scalar m11, Scalar m12,
        Scalar m20, Scalar m21, Scalar m22) noexcept
        : elements_{
              m00, m01, m02,
              m10, m11, m12,
              m20, m21, m22}
    {
    }

    [[nodiscard]] static constexpr Matrix3 identity() noexcept
    {
        return {
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0};
    }

    [[nodiscard]] constexpr Scalar operator()(std::size_t row, std::size_t column) const
    {
        if (row >= dimension || column >= dimension)
        {
            throw std::out_of_range{"Matrix3 index is out of range"};
        }

        return elements_[row * dimension + column];
    }

    [[nodiscard]] constexpr Matrix3 transposed() const
    {
        return {
            (*this)(0, 0), (*this)(1, 0), (*this)(2, 0),
            (*this)(0, 1), (*this)(1, 1), (*this)(2, 1),
            (*this)(0, 2), (*this)(1, 2), (*this)(2, 2)};
    }

    [[nodiscard]] constexpr Scalar determinant() const
    {
        const Scalar a = (*this)(0, 0);
        const Scalar b = (*this)(0, 1);
        const Scalar c = (*this)(0, 2);
        const Scalar d = (*this)(1, 0);
        const Scalar e = (*this)(1, 1);
        const Scalar f = (*this)(1, 2);
        const Scalar g = (*this)(2, 0);
        const Scalar h = (*this)(2, 1);
        const Scalar i = (*this)(2, 2);

        return a * (e * i - f * h)
            - b * (d * i - f * g)
            + c * (d * h - e * g);
    }

private:
    static constexpr std::size_t dimension = 3;
    std::array<Scalar, dimension * dimension> elements_{};
};

[[nodiscard]] constexpr Vector3 operator*(const Matrix3& matrix, const Vector3& vector)
{
    return {
        matrix(0, 0) * vector.x() + matrix(0, 1) * vector.y() + matrix(0, 2) * vector.z(),
        matrix(1, 0) * vector.x() + matrix(1, 1) * vector.y() + matrix(1, 2) * vector.z(),
        matrix(2, 0) * vector.x() + matrix(2, 1) * vector.y() + matrix(2, 2) * vector.z()};
}

[[nodiscard]] constexpr Matrix3 operator*(const Matrix3& first, const Matrix3& second)
{
    return {
        first(0, 0) * second(0, 0) + first(0, 1) * second(1, 0) + first(0, 2) * second(2, 0),
        first(0, 0) * second(0, 1) + first(0, 1) * second(1, 1) + first(0, 2) * second(2, 1),
        first(0, 0) * second(0, 2) + first(0, 1) * second(1, 2) + first(0, 2) * second(2, 2),
        first(1, 0) * second(0, 0) + first(1, 1) * second(1, 0) + first(1, 2) * second(2, 0),
        first(1, 0) * second(0, 1) + first(1, 1) * second(1, 1) + first(1, 2) * second(2, 1),
        first(1, 0) * second(0, 2) + first(1, 1) * second(1, 2) + first(1, 2) * second(2, 2),
        first(2, 0) * second(0, 0) + first(2, 1) * second(1, 0) + first(2, 2) * second(2, 0),
        first(2, 0) * second(0, 1) + first(2, 1) * second(1, 1) + first(2, 2) * second(2, 1),
        first(2, 0) * second(0, 2) + first(2, 1) * second(1, 2) + first(2, 2) * second(2, 2)};
}

[[nodiscard]] bool almostEqual(
    const Matrix3& first,
    const Matrix3& second,
    Scalar absoluteTolerance = defaultAbsoluteTolerance,
    Scalar relativeTolerance = defaultRelativeTolerance);

}
