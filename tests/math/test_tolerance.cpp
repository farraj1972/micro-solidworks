#include "core/math/Scalar.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
#include <type_traits>

namespace
{

using microsw::math::Scalar;
using microsw::math::almostEqual;
using microsw::math::defaultAbsoluteTolerance;
using microsw::math::defaultRelativeTolerance;
using microsw::math::isNearlyZero;

static_assert(std::is_same_v<Scalar, double>);

TEST(Tolerance, ExactlyEqualValuesCompareEqual)
{
    EXPECT_TRUE(almostEqual(42.0, 42.0));
    EXPECT_TRUE(almostEqual(-42.0, -42.0));
}

TEST(Tolerance, AbsoluteToleranceHandlesValuesNearZero)
{
    EXPECT_TRUE(almostEqual(0.0, defaultAbsoluteTolerance * 0.5));
    EXPECT_TRUE(almostEqual(-defaultAbsoluteTolerance * 0.5, 0.0));
    EXPECT_FALSE(almostEqual(0.0, defaultAbsoluteTolerance * 2.0));
}

TEST(Tolerance, RelativeToleranceHandlesLargeMagnitudes)
{
    constexpr Scalar value = 1.0e12;

    EXPECT_TRUE(almostEqual(value, value + 0.5));
    EXPECT_TRUE(almostEqual(-value, -value - 0.5));
    EXPECT_FALSE(almostEqual(value, value + 2.0));
}

TEST(Tolerance, ClearlyDifferentValuesDoNotCompareEqual)
{
    EXPECT_FALSE(almostEqual(1.0, 1.1));
    EXPECT_FALSE(almostEqual(-1.0, 1.0));
}

TEST(Tolerance, NearlyZeroUsesOnlyAbsoluteTolerance)
{
    EXPECT_TRUE(isNearlyZero(0.0));
    EXPECT_TRUE(isNearlyZero(defaultAbsoluteTolerance));
    EXPECT_TRUE(isNearlyZero(-defaultAbsoluteTolerance));
    EXPECT_FALSE(isNearlyZero(defaultAbsoluteTolerance * 2.0));
}

TEST(Tolerance, NegativeTolerancesAreRejected)
{
    EXPECT_THROW(
        static_cast<void>(almostEqual(1.0, 1.0, -1.0, defaultRelativeTolerance)),
        std::invalid_argument);
    EXPECT_THROW(
        static_cast<void>(almostEqual(1.0, 1.0, defaultAbsoluteTolerance, -1.0)),
        std::invalid_argument);
    EXPECT_THROW(static_cast<void>(isNearlyZero(0.0, -1.0)), std::invalid_argument);
}

TEST(Tolerance, NaNDoesNotCompareEqualOrNearlyZero)
{
    const Scalar nan = std::numeric_limits<Scalar>::quiet_NaN();

    EXPECT_FALSE(almostEqual(nan, nan));
    EXPECT_FALSE(almostEqual(nan, 0.0));
    EXPECT_FALSE(almostEqual(0.0, nan));
    EXPECT_FALSE(isNearlyZero(nan));
}

TEST(Tolerance, InfinityBehaviorIsExplicit)
{
    const Scalar positiveInfinity = std::numeric_limits<Scalar>::infinity();
    const Scalar negativeInfinity = -positiveInfinity;

    EXPECT_TRUE(almostEqual(positiveInfinity, positiveInfinity));
    EXPECT_TRUE(almostEqual(negativeInfinity, negativeInfinity));
    EXPECT_FALSE(almostEqual(positiveInfinity, 1.0));
    EXPECT_FALSE(almostEqual(positiveInfinity, negativeInfinity));
    EXPECT_FALSE(isNearlyZero(positiveInfinity));
    EXPECT_FALSE(isNearlyZero(negativeInfinity));
}

}
