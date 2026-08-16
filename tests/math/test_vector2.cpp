#include "core/math/Tolerance.h"
#include "core/math/Vector2.h"

#include <gtest/gtest.h>

#include <stdexcept>

namespace
{

using microsw::math::Scalar;
using microsw::math::Vector2;
using microsw::math::almostEqual;
using microsw::math::defaultAbsoluteTolerance;
using microsw::math::dot;

TEST(Vector2, DefaultConstructionProducesZeroVector)
{
    constexpr Vector2 vector{};

    static_assert(vector.x() == Scalar{0.0});
    static_assert(vector.y() == Scalar{0.0});
}

TEST(Vector2, ExplicitConstructionAndAccessPreserveComponents)
{
    constexpr Vector2 vector{3.0, -4.0};

    static_assert(vector.x() == Scalar{3.0});
    static_assert(vector.y() == Scalar{-4.0});
}

TEST(Vector2, AdditionSubtractionAndNegationAreComponentWise)
{
    constexpr Vector2 first{1.0, -2.0};
    constexpr Vector2 second{3.0, 4.0};

    constexpr Vector2 sum = first + second;
    constexpr Vector2 difference = first - second;
    constexpr Vector2 negated = -first;

    static_assert(sum.x() == Scalar{4.0} && sum.y() == Scalar{2.0});
    static_assert(difference.x() == Scalar{-2.0} && difference.y() == Scalar{-6.0});
    static_assert(negated.x() == Scalar{-1.0} && negated.y() == Scalar{2.0});
}

TEST(Vector2, ScalarMultiplicationSupportsBothOrders)
{
    constexpr Vector2 vector{2.0, -3.0};
    constexpr Vector2 rightProduct = vector * Scalar{4.0};
    constexpr Vector2 leftProduct = Scalar{4.0} * vector;

    static_assert(rightProduct.x() == Scalar{8.0} && rightProduct.y() == Scalar{-12.0});
    static_assert(leftProduct.x() == Scalar{8.0} && leftProduct.y() == Scalar{-12.0});
}

TEST(Vector2, ScalarDivisionDividesBothComponents)
{
    const Vector2 quotient = Vector2{8.0, -12.0} / Scalar{4.0};

    EXPECT_TRUE(almostEqual(quotient, Vector2{2.0, -3.0}));
}

TEST(Vector2, DivisionRejectsZeroAndNearlyZeroScalars)
{
    const Vector2 vector{1.0, 2.0};

    EXPECT_THROW(static_cast<void>(vector / Scalar{0.0}), std::domain_error);
    EXPECT_THROW(
        static_cast<void>(vector / (defaultAbsoluteTolerance * Scalar{0.5})),
        std::domain_error);
}

TEST(Vector2, SquaredLengthAndLengthMatchThreeFourFiveTriangle)
{
    constexpr Vector2 vector{3.0, 4.0};

    static_assert(vector.squaredLength() == Scalar{25.0});
    EXPECT_DOUBLE_EQ(vector.length(), 5.0);
}

TEST(Vector2, NormalizedReturnsUnitLengthCopy)
{
    const Vector2 original{3.0, 4.0};
    const Vector2 normalized = original.normalized();

    EXPECT_TRUE(almostEqual(normalized, Vector2{0.6, 0.8}));
    EXPECT_TRUE(microsw::math::almostEqual(normalized.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(original, Vector2{3.0, 4.0}));
}

TEST(Vector2, NormalizationRejectsZeroAndNearlyZeroVectors)
{
    const Vector2 zero{};
    const Vector2 nearlyZero{defaultAbsoluteTolerance * Scalar{0.5}, 0.0};

    EXPECT_THROW(static_cast<void>(zero.normalized()), std::domain_error);
    EXPECT_THROW(static_cast<void>(nearlyZero.normalized()), std::domain_error);
}

TEST(Vector2, DotProductMatchesDefinition)
{
    constexpr Scalar product = dot(Vector2{1.0, 2.0}, Vector2{3.0, 4.0});
    constexpr Scalar orthogonalProduct = dot(Vector2{1.0, 0.0}, Vector2{0.0, 1.0});

    static_assert(product == Scalar{11.0});
    static_assert(orthogonalProduct == Scalar{0.0});
}

TEST(Vector2, ApproximateEqualityIsExplicitAndComponentWise)
{
    const Vector2 reference{1.0, -2.0};
    const Vector2 close{
        1.0 + defaultAbsoluteTolerance * Scalar{0.5},
        -2.0 - defaultAbsoluteTolerance * Scalar{0.5}};

    EXPECT_TRUE(almostEqual(reference, close));
    EXPECT_FALSE(almostEqual(reference, Vector2{1.0, -2.1}));
}

}
