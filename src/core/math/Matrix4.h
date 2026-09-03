#pragma once

#include "core/math/Scalar.h"
#include "core/math/Tolerance.h"

#include <array>
#include <cstddef>
#include <stdexcept>

namespace microsw::math
{

class Matrix4
{
public:
    constexpr Matrix4() = default;

    // Arguments describe rows for readability; physical storage is private.
    constexpr Matrix4(
        Scalar m00, Scalar m01, Scalar m02, Scalar m03,
        Scalar m10, Scalar m11, Scalar m12, Scalar m13,
        Scalar m20, Scalar m21, Scalar m22, Scalar m23,
        Scalar m30, Scalar m31, Scalar m32, Scalar m33) noexcept
        : elements_{
              m00, m01, m02, m03,
              m10, m11, m12, m13,
              m20, m21, m22, m23,
              m30, m31, m32, m33}
    {
    }

    [[nodiscard]] static constexpr Matrix4 identity() noexcept
    {
        return {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0};
    }

    [[nodiscard]] constexpr Scalar operator()(std::size_t row, std::size_t column) const
    {
        if (row >= dimension || column >= dimension)
        {
            throw std::out_of_range{"Matrix4 index is out of range"};
        }

        return elements_[row * dimension + column];
    }

    [[nodiscard]] constexpr Matrix4 transposed() const
    {
        return {
            (*this)(0, 0), (*this)(1, 0), (*this)(2, 0), (*this)(3, 0),
            (*this)(0, 1), (*this)(1, 1), (*this)(2, 1), (*this)(3, 1),
            (*this)(0, 2), (*this)(1, 2), (*this)(2, 2), (*this)(3, 2),
            (*this)(0, 3), (*this)(1, 3), (*this)(2, 3), (*this)(3, 3)};
    }

    friend constexpr Matrix4 operator*(const Matrix4& first, const Matrix4& second);

private:
    static constexpr std::size_t dimension = 4;
    std::array<Scalar, dimension * dimension> elements_{};
};

[[nodiscard]] constexpr Matrix4 operator*(const Matrix4& first, const Matrix4& second)
{
    Matrix4 result{};
    for (std::size_t row = 0; row < Matrix4::dimension; ++row)
    {
        for (std::size_t column = 0; column < Matrix4::dimension; ++column)
        {
            Scalar sum{};
            for (std::size_t k = 0; k < Matrix4::dimension; ++k)
            {
                sum += first(row, k) * second(k, column);
            }

            result.elements_[row * Matrix4::dimension + column] = sum;
        }
    }

    return result;
}

[[nodiscard]] bool almostEqual(
    const Matrix4& first,
    const Matrix4& second,
    Scalar absoluteTolerance = defaultAbsoluteTolerance,
    Scalar relativeTolerance = defaultRelativeTolerance);

}
