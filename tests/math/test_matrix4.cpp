#include "core/math/Matrix4.h"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{

using microsw::math::Matrix4;
using microsw::math::Scalar;
using microsw::math::almostEqual;
using microsw::math::defaultAbsoluteTolerance;

constexpr Matrix4 sampleMatrix{
    1.0, 2.0, 3.0, 4.0,
    5.0, 6.0, 7.0, 8.0,
    9.0, 10.0, 11.0, 12.0,
    13.0, 14.0, 15.0, 16.0};

void expectElementsEqual(const Matrix4& actual, const Matrix4& expected)
{
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            EXPECT_DOUBLE_EQ(actual(row, column), expected(row, column))
                << "row=" << row << ", column=" << column;
        }
    }
}

TEST(Matrix4, DefaultConstructionProducesZeroMatrix)
{
    constexpr Matrix4 matrix{};
    static_assert(matrix(0, 0) == Scalar{0.0});
    static_assert(matrix(3, 3) == Scalar{0.0});

    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            EXPECT_DOUBLE_EQ(matrix(row, column), Scalar{0.0});
        }
    }
}

TEST(Matrix4, ExplicitConstructionUsesSemanticRowColumnOrder)
{
    static_assert(sampleMatrix(0, 3) == Scalar{4.0});
    static_assert(sampleMatrix(3, 0) == Scalar{13.0});

    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            EXPECT_DOUBLE_EQ(
                sampleMatrix(row, column), static_cast<Scalar>(row * 4 + column + 1));
        }
    }
}

TEST(Matrix4, AccessRejectsOutOfRangeRow)
{
    EXPECT_THROW(static_cast<void>(sampleMatrix(4, 0)), std::out_of_range);
    EXPECT_THROW(
        static_cast<void>(sampleMatrix(std::numeric_limits<std::size_t>::max(), 0)),
        std::out_of_range);
}

TEST(Matrix4, AccessRejectsOutOfRangeColumn)
{
    EXPECT_THROW(static_cast<void>(sampleMatrix(0, 4)), std::out_of_range);
    EXPECT_THROW(
        static_cast<void>(sampleMatrix(0, std::numeric_limits<std::size_t>::max())),
        std::out_of_range);
}

TEST(Matrix4, IdentityHasAllExpectedElements)
{
    constexpr Matrix4 identity = Matrix4::identity();
    static_assert(identity(3, 3) == Scalar{1.0});

    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            EXPECT_DOUBLE_EQ(identity(row, column), row == column ? Scalar{1.0} : Scalar{0.0});
        }
    }
}

TEST(Matrix4, LeftIdentityPreservesMatrix)
{
    constexpr Matrix4 result = Matrix4::identity() * sampleMatrix;
    static_assert(result(3, 2) == Scalar{15.0});
    expectElementsEqual(result, sampleMatrix);
}

TEST(Matrix4, RightIdentityPreservesMatrix)
{
    constexpr Matrix4 result = sampleMatrix * Matrix4::identity();
    static_assert(result(2, 3) == Scalar{12.0});
    expectElementsEqual(result, sampleMatrix);
}

TEST(Matrix4, MatrixProductMatchesRowColumnDefinition)
{
    constexpr Matrix4 second{
        1.0, 0.0, 2.0, 0.0,
        0.0, 1.0, 0.0, 3.0,
        4.0, 0.0, 1.0, 0.0,
        0.0, 5.0, 0.0, 1.0};
    // Each output column combines two columns of sampleMatrix.
    constexpr Matrix4 expected{
        13.0, 22.0, 5.0, 10.0,
        33.0, 46.0, 17.0, 26.0,
        53.0, 70.0, 29.0, 42.0,
        73.0, 94.0, 41.0, 58.0};
    constexpr Matrix4 result = sampleMatrix * second;

    static_assert(result(0, 0) == Scalar{13.0});
    static_assert(result(3, 3) == Scalar{58.0});
    expectElementsEqual(result, expected);
}

TEST(Matrix4, ZeroMatrixAnnihilatesBothProducts)
{
    expectElementsEqual(sampleMatrix * Matrix4{}, Matrix4{});
    expectElementsEqual(Matrix4{} * sampleMatrix, Matrix4{});
}

TEST(Matrix4, TransposeSwapsAllRowsAndColumns)
{
    constexpr Matrix4 result = sampleMatrix.transposed();
    constexpr Matrix4 expected{
        1.0, 5.0, 9.0, 13.0,
        2.0, 6.0, 10.0, 14.0,
        3.0, 7.0, 11.0, 15.0,
        4.0, 8.0, 12.0, 16.0};

    static_assert(result(0, 3) == Scalar{13.0});
    expectElementsEqual(result, expected);
    EXPECT_DOUBLE_EQ(sampleMatrix(0, 3), Scalar{4.0});
}

TEST(Matrix4, DoubleTransposeReturnsOriginal)
{
    constexpr Matrix4 result = sampleMatrix.transposed().transposed();
    static_assert(result(3, 0) == Scalar{13.0});
    expectElementsEqual(result, sampleMatrix);
}

TEST(Matrix4, ApproximateEqualityChecksEveryElement)
{
    // Construct through the public API, changing one semantic element per case.
    for (std::size_t changed = 0; changed < 16; ++changed)
    {
        const auto value = [changed](std::size_t index, Scalar delta)
        {
            return static_cast<Scalar>(index + 1) + (index == changed ? delta : Scalar{0.0});
        };
        const auto perturbed = [&value](Scalar delta)
        {
            return Matrix4{
                value(0, delta), value(1, delta), value(2, delta), value(3, delta),
                value(4, delta), value(5, delta), value(6, delta), value(7, delta),
                value(8, delta), value(9, delta), value(10, delta), value(11, delta),
                value(12, delta), value(13, delta), value(14, delta), value(15, delta)};
        };

        EXPECT_TRUE(almostEqual(sampleMatrix, perturbed(defaultAbsoluteTolerance * Scalar{0.5})));
        EXPECT_FALSE(almostEqual(sampleMatrix, perturbed(Scalar{0.1})));
    }
}

TEST(Matrix4, ApproximateEqualityForwardsCustomTolerances)
{
    const Matrix4 different{
        1.0, 2.0, 3.0, 4.0,
        5.0, 6.0, 7.0, 8.0,
        9.0, 10.0, 11.0, 12.0,
        13.0, 14.0, 15.0, 16.01};

    EXPECT_FALSE(almostEqual(sampleMatrix, different));
    EXPECT_TRUE(almostEqual(sampleMatrix, different, Scalar{0.02}, Scalar{0.0}));
    EXPECT_TRUE(almostEqual(sampleMatrix, different, Scalar{0.0}, Scalar{0.001}));
    EXPECT_THROW(
        static_cast<void>(almostEqual(sampleMatrix, different, Scalar{-1.0}, Scalar{0.0})),
        std::invalid_argument);
}

}
