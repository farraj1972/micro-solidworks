#include "core/math/Tolerance.h"
#include "core/math/Vector3.h"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{

using microsw::math::Scalar;
using microsw::math::Vector3;
using microsw::math::almostEqual;
using microsw::math::cross;
using microsw::math::defaultAbsoluteTolerance;
using microsw::math::dot;

TEST(Vector3, ConstructionAndAccessPreserveComponents)
{
    constexpr Vector3 zero{};
    constexpr Vector3 vector{2.0, -3.0, 6.0};

    static_assert(zero.x() == Scalar{0.0} && zero.y() == Scalar{0.0} && zero.z() == Scalar{0.0});
    static_assert(vector.x() == Scalar{2.0} && vector.y() == Scalar{-3.0}
        && vector.z() == Scalar{6.0});
}

TEST(Vector3, ArithmeticIsComponentWise)
{
    constexpr Vector3 first{1.0, -2.0, 3.0};
    constexpr Vector3 second{4.0, 5.0, -6.0};
    constexpr Vector3 sum = first + second;
    constexpr Vector3 difference = first - second;
    constexpr Vector3 negated = -first;

    static_assert(sum.x() == Scalar{5.0} && sum.y() == Scalar{3.0} && sum.z() == Scalar{-3.0});
    static_assert(difference.x() == Scalar{-3.0} && difference.y() == Scalar{-7.0}
        && difference.z() == Scalar{9.0});
    static_assert(negated.x() == Scalar{-1.0} && negated.y() == Scalar{2.0}
        && negated.z() == Scalar{-3.0});
}

TEST(Vector3, ScalarMultiplicationSupportsBothOrders)
{
    constexpr Vector3 vector{2.0, -3.0, 4.0};
    constexpr Vector3 right = vector * Scalar{2.0};
    constexpr Vector3 left = Scalar{2.0} * vector;

    static_assert(right.x() == Scalar{4.0} && right.y() == Scalar{-6.0} && right.z() == Scalar{8.0});
    static_assert(left.x() == Scalar{4.0} && left.y() == Scalar{-6.0} && left.z() == Scalar{8.0});
}

TEST(Vector3, ScalarDivisionUsesVector2Policy)
{
    EXPECT_TRUE(almostEqual(Vector3{8.0, -12.0, 16.0} / Scalar{4.0}, Vector3{2.0, -3.0, 4.0}));
    EXPECT_THROW(static_cast<void>(Vector3{1.0, 2.0, 3.0} / Scalar{0.0}), std::domain_error);
    EXPECT_THROW(
        static_cast<void>(Vector3{1.0, 2.0, 3.0}
            / (defaultAbsoluteTolerance * Scalar{0.5})),
        std::domain_error);
}

TEST(Vector3, SquaredLengthAndLengthMatchAuditableExample)
{
    constexpr Vector3 vector{2.0, 3.0, 6.0};

    static_assert(vector.squaredLength() == Scalar{49.0});
    EXPECT_DOUBLE_EQ(vector.length(), 7.0);
}

TEST(Vector3, NormalizedReturnsUnitLengthCopy)
{
    const Vector3 original{0.0, 3.0, 4.0};
    const Vector3 normalized = original.normalized();

    EXPECT_TRUE(almostEqual(normalized, Vector3{0.0, 0.6, 0.8}));
    EXPECT_TRUE(microsw::math::almostEqual(normalized.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(original, Vector3{0.0, 3.0, 4.0}));
}

TEST(Vector3, NormalizationRejectsZeroAndNearlyZeroVectors)
{
    EXPECT_THROW(static_cast<void>(Vector3{}.normalized()), std::domain_error);
    EXPECT_THROW(
        static_cast<void>(
            Vector3{defaultAbsoluteTolerance * Scalar{0.5}, 0.0, 0.0}.normalized()),
        std::domain_error);
}

TEST(Vector3, DotProductMatchesDefinitionAndOrthogonality)
{
    constexpr Scalar product = dot(Vector3{1.0, 2.0, 3.0}, Vector3{4.0, 5.0, 6.0});
    constexpr Scalar orthogonal = dot(Vector3{1.0, 0.0, 0.0}, Vector3{0.0, 1.0, 0.0});

    static_assert(product == Scalar{32.0});
    static_assert(orthogonal == Scalar{0.0});
}

TEST(Vector3, CrossProductMaterializesRightHandedBasis)
{
    constexpr Vector3 xAxis{1.0, 0.0, 0.0};
    constexpr Vector3 yAxis{0.0, 1.0, 0.0};
    constexpr Vector3 zAxis{0.0, 0.0, 1.0};

    static_assert(cross(xAxis, yAxis).x() == Scalar{0.0}
        && cross(xAxis, yAxis).y() == Scalar{0.0}
        && cross(xAxis, yAxis).z() == Scalar{1.0});
    static_assert(cross(yAxis, zAxis).x() == Scalar{1.0}
        && cross(yAxis, zAxis).y() == Scalar{0.0}
        && cross(yAxis, zAxis).z() == Scalar{0.0});
    static_assert(cross(zAxis, xAxis).x() == Scalar{0.0}
        && cross(zAxis, xAxis).y() == Scalar{1.0}
        && cross(zAxis, xAxis).z() == Scalar{0.0});
    static_assert(cross(yAxis, xAxis).x() == Scalar{0.0}
        && cross(yAxis, xAxis).y() == Scalar{0.0}
        && cross(yAxis, xAxis).z() == Scalar{-1.0});
}

TEST(Vector3, CrossProductWithSelfProducesZeroVector)
{
    constexpr Vector3 result = cross(Vector3{1.0, 2.0, 3.0}, Vector3{1.0, 2.0, 3.0});

    static_assert(result.x() == Scalar{0.0} && result.y() == Scalar{0.0}
        && result.z() == Scalar{0.0});
}

TEST(Vector3, ApproximateEqualityIsExplicitAndComponentWise)
{
    const Vector3 reference{1.0, -2.0, 3.0};
    const Vector3 close{
        1.0 + defaultAbsoluteTolerance * Scalar{0.5},
        -2.0 - defaultAbsoluteTolerance * Scalar{0.5},
        3.0 + defaultAbsoluteTolerance * Scalar{0.5}};

    EXPECT_TRUE(almostEqual(reference, close));
    EXPECT_FALSE(almostEqual(reference, Vector3{1.0, -2.0, 3.1}));
}

}
