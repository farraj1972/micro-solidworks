#include "core/math/Matrix3.h"
#include "core/math/Tolerance.h"
#include "core/math/Vector3.h"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{

using microsw::math::Matrix3;
using microsw::math::Scalar;
using microsw::math::Vector3;
using microsw::math::almostEqual;
using microsw::math::defaultAbsoluteTolerance;

constexpr Matrix3 sampleMatrix{
    1.0, 2.0, 3.0,
    4.0, 5.0, 6.0,
    7.0, 8.0, 9.0};

TEST(Matrix3, DefaultConstructionProducesZeroMatrix)
{
    constexpr Matrix3 matrix{};

    static_assert(matrix(0, 0) == Scalar{0.0});
    static_assert(matrix(1, 1) == Scalar{0.0});
    static_assert(matrix(2, 2) == Scalar{0.0});
}

TEST(Matrix3, ExplicitConstructionUsesSemanticRowMajorOrder)
{
    static_assert(sampleMatrix(0, 0) == Scalar{1.0});
    static_assert(sampleMatrix(0, 2) == Scalar{3.0});
    static_assert(sampleMatrix(1, 0) == Scalar{4.0});
    static_assert(sampleMatrix(1, 2) == Scalar{6.0});
    static_assert(sampleMatrix(2, 0) == Scalar{7.0});
    static_assert(sampleMatrix(2, 2) == Scalar{9.0});
}

TEST(Matrix3, ElementAccessRejectsOutOfRangeRowsAndColumns)
{
    EXPECT_THROW(static_cast<void>(sampleMatrix(3, 0)), std::out_of_range);
    EXPECT_THROW(static_cast<void>(sampleMatrix(0, 3)), std::out_of_range);
}

TEST(Matrix3, IdentityHasExpectedElements)
{
    constexpr Matrix3 identity = Matrix3::identity();

    static_assert(identity(0, 0) == Scalar{1.0});
    static_assert(identity(1, 1) == Scalar{1.0});
    static_assert(identity(2, 2) == Scalar{1.0});
    static_assert(identity(0, 1) == Scalar{0.0});
    static_assert(identity(1, 2) == Scalar{0.0});
    static_assert(identity(2, 0) == Scalar{0.0});
}

TEST(Matrix3, IdentityPreservesVectorsAndMatrices)
{
    constexpr Matrix3 identity = Matrix3::identity();
    constexpr Vector3 vector{2.0, -3.0, 4.0};

    static_assert((identity * vector).x() == vector.x());
    static_assert((identity * vector).y() == vector.y());
    static_assert((identity * vector).z() == vector.z());
    static_assert((identity * sampleMatrix)(1, 2) == sampleMatrix(1, 2));
    static_assert((sampleMatrix * identity)(2, 1) == sampleMatrix(2, 1));

    EXPECT_TRUE(almostEqual(identity * sampleMatrix, sampleMatrix));
    EXPECT_TRUE(almostEqual(sampleMatrix * identity, sampleMatrix));
}

TEST(Matrix3, MatrixVectorProductUsesColumnVectorSemantics)
{
    constexpr Vector3 firstColumn = sampleMatrix * Vector3{1.0, 0.0, 0.0};
    constexpr Matrix3 matrix{
        1.0, 2.0, 3.0,
        0.0, 1.0, 4.0,
        5.0, 6.0, 0.0};
    constexpr Vector3 result = matrix * Vector3{1.0, 2.0, 3.0};

    static_assert(firstColumn.x() == Scalar{1.0});
    static_assert(firstColumn.y() == Scalar{4.0});
    static_assert(firstColumn.z() == Scalar{7.0});
    static_assert(result.x() == Scalar{14.0});
    static_assert(result.y() == Scalar{14.0});
    static_assert(result.z() == Scalar{17.0});
}

TEST(Matrix3, MatrixProductMatchesDefinition)
{
    constexpr Matrix3 second{
        9.0, 8.0, 7.0,
        6.0, 5.0, 4.0,
        3.0, 2.0, 1.0};
    constexpr Matrix3 expected{
        30.0, 24.0, 18.0,
        84.0, 69.0, 54.0,
        138.0, 114.0, 90.0};

    static_assert((sampleMatrix * second)(0, 0) == Scalar{30.0});
    EXPECT_TRUE(almostEqual(sampleMatrix * second, expected));
}

TEST(Matrix3, TransposeSwapsRowsAndColumns)
{
    constexpr Matrix3 transposed = sampleMatrix.transposed();
    constexpr Matrix3 expected{
        1.0, 4.0, 7.0,
        2.0, 5.0, 8.0,
        3.0, 6.0, 9.0};

    static_assert(transposed(0, 1) == Scalar{4.0});
    EXPECT_TRUE(almostEqual(transposed, expected));
    EXPECT_TRUE(almostEqual(transposed.transposed(), sampleMatrix));
}

TEST(Matrix3, DeterminantHandlesIdentitySingularAndNonTrivialMatrices)
{
    constexpr Matrix3 nonSingular{
        1.0, 2.0, 3.0,
        0.0, 1.0, 4.0,
        5.0, 6.0, 0.0};

    static_assert(Matrix3::identity().determinant() == Scalar{1.0});
    static_assert(sampleMatrix.determinant() == Scalar{0.0});
    static_assert(nonSingular.determinant() == Scalar{1.0});
}

TEST(Matrix3, ApproximateEqualityIsExplicitAndElementWise)
{
    const Matrix3 close{
        1.0, 2.0, 3.0,
        4.0, 5.0 + defaultAbsoluteTolerance * Scalar{0.5}, 6.0,
        7.0, 8.0, 9.0};

    EXPECT_TRUE(almostEqual(sampleMatrix, close));
    EXPECT_FALSE(almostEqual(
        sampleMatrix,
        Matrix3{
            1.0, 2.0, 3.0,
            4.0, 5.1, 6.0,
            7.0, 8.0, 9.0}));
}

}
