#include "core/geometry/Segment2.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
using microsw::geometry::Point2;
using microsw::geometry::Segment2;
using microsw::geometry::areCoincident;
using microsw::geometry::defaultGeometricTolerance;
using microsw::math::Scalar;
using microsw::math::Vector2;

template<class T> concept HasEquality = requires(T value) { value == value; };
static_assert(!std::is_base_of_v<Point2, Segment2>);
static_assert(!std::is_base_of_v<Vector2, Segment2>);
static_assert(!HasEquality<Segment2>);
static_assert(std::is_same_v<decltype(std::declval<Segment2&>().a()), const Point2&>);
static_assert(std::is_same_v<decltype(std::declval<Segment2&>().b()), const Point2&>);
static_assert(std::is_same_v<decltype(std::declval<Segment2>().direction()), Vector2>);
static_assert(std::is_copy_assignable_v<Segment2> && std::is_move_constructible_v<Segment2>);

TEST(Segment2, DefaultIsDegenerateOrigin)
{
    constexpr Segment2 segment{};
    static_assert(segment.a().x() == 0 && segment.b().x() == 0);
    EXPECT_TRUE(areCoincident(segment.a(), Point2{}, 0));
    EXPECT_TRUE(areCoincident(segment.b(), Point2{}, 0));
    EXPECT_TRUE(segment.isDegenerate());
    EXPECT_DOUBLE_EQ(segment.length(), 0);
    EXPECT_DOUBLE_EQ(segment.squaredLength(), 0);
    EXPECT_THROW((void)segment.direction(), std::domain_error);
}

TEST(Segment2, EndpointsAreOrderedIndependentValues)
{
    Point2 a{1, 1}, b{3, 4};
    const Segment2 segment{a, b};
    EXPECT_TRUE(areCoincident(segment.a(), a, 0));
    EXPECT_TRUE(areCoincident(segment.b(), b, 0));
    auto copy = segment;
    copy = Segment2{b, a};
    a = Point2{};
    EXPECT_TRUE(areCoincident(segment.a(), Point2{1, 1}, 0));
    EXPECT_TRUE(areCoincident(copy.a(), b, 0));
    (void)segment.length();
    (void)segment.squaredLength();
    (void)segment.midpoint();
    (void)segment.direction();
    EXPECT_TRUE(areCoincident(segment.b(), b, 0));
    EXPECT_TRUE(areCoincident(b, Point2{3, 4}, 0));
}

TEST(Segment2, EveryAxisLengthAndDirectionRespectOrder)
{
    for (std::size_t axis = 0; axis < 2; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 2> values{};
            values[axis] = 4 * sign;
            const Point2 endpoint{values[0], values[1]};
            const Segment2 segment{Point2{}, endpoint}, reverse{endpoint, Point2{}};
            EXPECT_DOUBLE_EQ(segment.length(), 4);
            EXPECT_DOUBLE_EQ(segment.squaredLength(), 16);
            EXPECT_TRUE(microsw::math::almostEqual(segment.direction(),
                Vector2{values[0] / 4, values[1] / 4}));
            EXPECT_TRUE(microsw::math::almostEqual(reverse.direction(), -segment.direction()));
        }
}

TEST(Segment2, DiagonalLengthDirectionAndMidpoint)
{
    const Point2 a{}, b{3, 4};
    const Segment2 segment{a, b}, reverse{b, a};
    EXPECT_NEAR(segment.length(), 5, 1e-12);
    EXPECT_NEAR(segment.squaredLength(), 25, 1e-12);
    EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
    EXPECT_TRUE(microsw::math::almostEqual(segment.direction(), Vector2{3.0 / 5, 4.0 / 5}));
    EXPECT_TRUE(microsw::math::almostEqual(reverse.direction(), -segment.direction()));
    EXPECT_TRUE(areCoincident(segment.midpoint(), Point2{1.5, 2}, 0));
    EXPECT_TRUE(areCoincident(reverse.midpoint(), segment.midpoint(), 0));
    EXPECT_NEAR(reverse.length(), segment.length(), 1e-12);
}

TEST(Segment2, TranslatedAndDegenerateMidpoints)
{
    const Point2 a{-4, -4}, b{2, 2};
    EXPECT_TRUE(areCoincident((Segment2{a, b}).midpoint(), Point2{-1, -1}, 0));
    EXPECT_TRUE(areCoincident((Segment2{a, a}).midpoint(), a, 0));
    EXPECT_THROW((void)(Segment2{a, a}).direction(), std::domain_error);
}

TEST(Segment2, GeometricThresholdAppliesInEveryAxis)
{
    static_assert(defaultGeometricTolerance == 1e-9);
    EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
    for (std::size_t axis = 0; axis < 2; ++axis)
        for (Scalar value : {5e-10, 1e-9, std::nextafter(1e-9, 1.0)})
        {
            std::array<Scalar, 2> coordinates{};
            coordinates[axis] = value;
            const Segment2 segment{Point2{}, Point2{coordinates[0], coordinates[1]}};
            EXPECT_EQ(segment.isDegenerate(), value <= 1e-9);
            if (value <= 1e-9)
                EXPECT_THROW((void)segment.direction(), std::domain_error);
            else
                EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
        }
}

TEST(Segment2, DegeneracyUsesEuclideanDistance)
{
    const Segment2 inside{Point2{}, Point2{0.5e-9, 0.5e-9}};
    const Segment2 outside{Point2{}, Point2{0.8e-9, 0.8e-9}};
    EXPECT_TRUE(inside.isDegenerate());
    EXPECT_FALSE(outside.isDegenerate());
    EXPECT_THROW((void)inside.direction(), std::domain_error);
    EXPECT_NEAR(outside.direction().length(), 1, 1e-12);
}

TEST(Segment2, CustomAndZeroTolerance)
{
    const Segment2 segment{Point2{}, Point2{3, 4}};
    EXPECT_TRUE(segment.isDegenerate(5));
    EXPECT_FALSE(segment.isDegenerate(5-0.01));
    EXPECT_FALSE(segment.isDegenerate(0));
    EXPECT_TRUE(Segment2{}.isDegenerate(0));
    // A previous custom query does not change direction's default threshold.
    EXPECT_NO_THROW((void)segment.direction());
    const auto tiny = std::numeric_limits<Scalar>::denorm_min();
    const Segment2 tinySegment{Point2{}, Point2{tiny, 0}};
    EXPECT_FALSE(tinySegment.isDegenerate(0));
    EXPECT_TRUE(tinySegment.isDegenerate());
}

TEST(Segment2, InvalidTolerancesThrowEvenForZeroLength)
{
    for (Scalar tolerance : {-1.0, -std::numeric_limits<Scalar>::denorm_min(),
         std::numeric_limits<Scalar>::quiet_NaN(),
         std::numeric_limits<Scalar>::infinity(), -std::numeric_limits<Scalar>::infinity()})
    {
        EXPECT_THROW((void)Segment2{}.isDegenerate(tolerance), std::invalid_argument);
        const Segment2 segment{Point2{}, Point2{3, 4}};
        EXPECT_THROW((void)segment.isDegenerate(tolerance), std::invalid_argument);
    }
}

TEST(Segment2, LargePositionDoesNotImplyRelativeDegeneracy)
{
    const Segment2 segment{Point2{1e12, 1e12}, Point2{1e12 + 0.5, 1e12}};
    EXPECT_FALSE(segment.isDegenerate());
    EXPECT_DOUBLE_EQ(segment.length(), 0.5);
    EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
}

TEST(Segment2, LargeFiniteLengthDoesNotSquareIntermediates)
{
    const Segment2 segment{Point2{}, Point2{1e200, 1e200}};
    EXPECT_NEAR(segment.length() / 1e200, std::sqrt(Scalar{2}), 1e-12);
    EXPECT_THROW((void)segment.squaredLength(), std::overflow_error);
    EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
}

TEST(Segment2, SquaredLengthCanRemainFiniteNearMaximum)
{
    const Segment2 segment{Point2{}, Point2{1e154, 0}};
    EXPECT_NEAR(segment.squaredLength() / 1e308, 1, 1e-12);
}

TEST(Segment2, TinyLengthAvoidsIntermediateUnderflow)
{
    const Segment2 segment{Point2{}, Point2{1e-200, 1e-200}};
    EXPECT_NEAR(segment.length() / 1e-200, std::sqrt(Scalar{2}), 1e-12);
    EXPECT_DOUBLE_EQ(segment.squaredLength(), 0); // Rounded underflow is finite.
    EXPECT_TRUE(segment.isDegenerate());
}

TEST(Segment2, OverflowingDifferenceStillHasFiniteMidpointAndDirection)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (std::size_t axis = 0; axis < 2; ++axis)
    {
        std::array<Scalar, 2> values{}, negatives{};
        values[axis] = maximum;
        negatives[axis] = -maximum;
        const Point2 a{negatives[0], negatives[1]}, b{values[0], values[1]};
        const Segment2 segment{a, b}, reverse{b, a};
        EXPECT_THROW((void)segment.length(), std::overflow_error);
        EXPECT_THROW((void)segment.squaredLength(), std::overflow_error);
        EXPECT_FALSE(segment.isDegenerate(maximum));
        EXPECT_TRUE(areCoincident(segment.midpoint(), Point2{}, 0));
        EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
        EXPECT_TRUE(microsw::math::almostEqual(reverse.direction(), -segment.direction()));
    }
}

TEST(Segment2, OverflowingNormStillHasUnitDirection)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Segment2 segment{Point2{}, Point2{maximum, maximum}};
    EXPECT_THROW((void)segment.length(), std::overflow_error);
    EXPECT_THROW((void)segment.squaredLength(), std::overflow_error);
    const auto direction = segment.direction();
    EXPECT_NEAR(direction.x(), 1 / std::sqrt(Scalar{2}), 1e-12);
    EXPECT_NEAR(direction.y(), 1 / std::sqrt(Scalar{2}), 1e-12);
}

TEST(Segment2, OverflowScalingPreservesUnequalComponentRatios)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Segment2 segment{Point2{-maximum, 0},
        Point2{maximum, maximum}};
    const auto direction = segment.direction();
    EXPECT_NEAR(direction.y() / direction.x(), 0.5, 1e-12);
    EXPECT_NEAR(direction.length(), 1, 1e-12);
}

TEST(Segment2, MidpointHandlesSameSignExtremesAndSubnormals)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const auto tiny = std::numeric_limits<Scalar>::denorm_min();
    for (Scalar value : {maximum, -maximum, tiny, -tiny})
    {
        const Point2 endpoint{value, value};
        EXPECT_TRUE(areCoincident((Segment2{endpoint, endpoint}).midpoint(), endpoint, 0));
    }
    const Point2 a{maximum, maximum}, b{maximum / 2, maximum / 2};
    EXPECT_TRUE(areCoincident((Segment2{a, b}).midpoint(), Point2{maximum * 0.75, maximum * 0.75}, 0));
    EXPECT_TRUE(areCoincident((Segment2{b, a}).midpoint(), (Segment2{a, b}).midpoint(), 0));
}

}
