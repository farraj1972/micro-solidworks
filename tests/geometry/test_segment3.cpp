#include "core/geometry/Segment3.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace
{
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::areCoincident;
using microsw::geometry::defaultGeometricTolerance;
using microsw::math::Scalar;
using microsw::math::Vector3;

template<class T> concept HasEquality = requires(T value) { value == value; };
static_assert(!std::is_base_of_v<Point3, Segment3>);
static_assert(!std::is_base_of_v<Vector3, Segment3>);
static_assert(!HasEquality<Segment3>);
static_assert(std::is_same_v<decltype(std::declval<Segment3&>().a()), const Point3&>);
static_assert(std::is_same_v<decltype(std::declval<Segment3&>().b()), const Point3&>);
static_assert(std::is_same_v<decltype(std::declval<Segment3>().direction()), Vector3>);
static_assert(std::is_copy_assignable_v<Segment3> && std::is_move_constructible_v<Segment3>);

TEST(Segment3, DefaultIsDegenerateOrigin)
{
    constexpr Segment3 segment{};
    static_assert(segment.a().x() == 0 && segment.b().x() == 0);
    EXPECT_TRUE(areCoincident(segment.a(), Point3{}, 0));
    EXPECT_TRUE(areCoincident(segment.b(), Point3{}, 0));
    EXPECT_TRUE(segment.isDegenerate());
    EXPECT_DOUBLE_EQ(segment.length(), 0);
    EXPECT_DOUBLE_EQ(segment.squaredLength(), 0);
    EXPECT_THROW((void)segment.direction(), std::domain_error);
}

TEST(Segment3, EndpointsAreOrderedIndependentValues)
{
    Point3 a{1, 1, 1}, b{2, 3, 6};
    const Segment3 segment{a, b};
    EXPECT_TRUE(areCoincident(segment.a(), a, 0));
    EXPECT_TRUE(areCoincident(segment.b(), b, 0));
    auto copy = segment;
    copy = Segment3{b, a};
    a = Point3{};
    EXPECT_TRUE(areCoincident(segment.a(), Point3{1, 1, 1}, 0));
    EXPECT_TRUE(areCoincident(copy.a(), b, 0));
    (void)segment.length();
    (void)segment.squaredLength();
    (void)segment.midpoint();
    (void)segment.direction();
    EXPECT_TRUE(areCoincident(segment.b(), b, 0));
    EXPECT_TRUE(areCoincident(b, Point3{2, 3, 6}, 0));
}

TEST(Segment3, EveryAxisLengthAndDirectionRespectOrder)
{
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 3> values{};
            values[axis] = 4 * sign;
            const Point3 endpoint{values[0], values[1], values[2]};
            const Segment3 segment{Point3{}, endpoint}, reverse{endpoint, Point3{}};
            EXPECT_DOUBLE_EQ(segment.length(), 4);
            EXPECT_DOUBLE_EQ(segment.squaredLength(), 16);
            EXPECT_TRUE(microsw::math::almostEqual(segment.direction(),
                Vector3{values[0] / 4, values[1] / 4, values[2] / 4}));
            EXPECT_TRUE(microsw::math::almostEqual(reverse.direction(), -segment.direction()));
        }
}

TEST(Segment3, DiagonalLengthDirectionAndMidpoint)
{
    const Point3 a{}, b{2, 3, 6};
    const Segment3 segment{a, b}, reverse{b, a};
    EXPECT_NEAR(segment.length(), 7, 1e-12);
    EXPECT_NEAR(segment.squaredLength(), 49, 1e-12);
    EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
    EXPECT_TRUE(microsw::math::almostEqual(segment.direction(), Vector3{2.0 / 7, 3.0 / 7, 6.0 / 7}));
    EXPECT_TRUE(microsw::math::almostEqual(reverse.direction(), -segment.direction()));
    EXPECT_TRUE(areCoincident(segment.midpoint(), Point3{1, 1.5, 3}, 0));
    EXPECT_TRUE(areCoincident(reverse.midpoint(), segment.midpoint(), 0));
    EXPECT_NEAR(reverse.length(), segment.length(), 1e-12);
}

TEST(Segment3, TranslatedAndDegenerateMidpoints)
{
    const Point3 a{-4, -4, -4}, b{2, 2, 2};
    EXPECT_TRUE(areCoincident((Segment3{a, b}).midpoint(), Point3{-1, -1, -1}, 0));
    EXPECT_TRUE(areCoincident((Segment3{a, a}).midpoint(), a, 0));
    EXPECT_THROW((void)(Segment3{a, a}).direction(), std::domain_error);
}

TEST(Segment3, GeometricThresholdAppliesInEveryAxis)
{
    static_assert(defaultGeometricTolerance == 1e-9);
    EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar value : {5e-10, 1e-9, std::nextafter(1e-9, 1.0)})
        {
            std::array<Scalar, 3> coordinates{};
            coordinates[axis] = value;
            const Segment3 segment{Point3{}, Point3{coordinates[0], coordinates[1], coordinates[2]}};
            EXPECT_EQ(segment.isDegenerate(), value <= 1e-9);
            if (value <= 1e-9)
                EXPECT_THROW((void)segment.direction(), std::domain_error);
            else
                EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
        }
}

TEST(Segment3, DegeneracyUsesEuclideanDistance)
{
    const Segment3 inside{Point3{}, Point3{0.5e-9, 0.5e-9, 0.5e-9}};
    const Segment3 outside{Point3{}, Point3{0.8e-9, 0.8e-9, 0.8e-9}};
    EXPECT_TRUE(inside.isDegenerate());
    EXPECT_FALSE(outside.isDegenerate());
    EXPECT_THROW((void)inside.direction(), std::domain_error);
    EXPECT_NEAR(outside.direction().length(), 1, 1e-12);
}

TEST(Segment3, CustomAndZeroTolerance)
{
    const Segment3 segment{Point3{}, Point3{2, 3, 6}};
    EXPECT_TRUE(segment.isDegenerate(7));
    EXPECT_FALSE(segment.isDegenerate(7-0.01));
    EXPECT_FALSE(segment.isDegenerate(0));
    EXPECT_TRUE(Segment3{}.isDegenerate(0));
    // A previous custom query does not change direction's default threshold.
    EXPECT_NO_THROW((void)segment.direction());
    const auto tiny = std::numeric_limits<Scalar>::denorm_min();
    const Segment3 tinySegment{Point3{}, Point3{tiny, 0, 0}};
    EXPECT_FALSE(tinySegment.isDegenerate(0));
    EXPECT_TRUE(tinySegment.isDegenerate());
}

TEST(Segment3, InvalidTolerancesThrowEvenForZeroLength)
{
    for (Scalar tolerance : {-1.0, -std::numeric_limits<Scalar>::denorm_min(),
         std::numeric_limits<Scalar>::quiet_NaN(),
         std::numeric_limits<Scalar>::infinity(), -std::numeric_limits<Scalar>::infinity()})
    {
        EXPECT_THROW((void)Segment3{}.isDegenerate(tolerance), std::invalid_argument);
        const Segment3 segment{Point3{}, Point3{2, 3, 6}};
        EXPECT_THROW((void)segment.isDegenerate(tolerance), std::invalid_argument);
    }
}

TEST(Segment3, LargePositionDoesNotImplyRelativeDegeneracy)
{
    const Segment3 segment{Point3{1e12, 1e12, 1e12}, Point3{1e12 + 0.5, 1e12, 1e12}};
    EXPECT_FALSE(segment.isDegenerate());
    EXPECT_DOUBLE_EQ(segment.length(), 0.5);
    EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
}

TEST(Segment3, LargeFiniteLengthDoesNotSquareIntermediates)
{
    const Segment3 segment{Point3{}, Point3{1e200, 1e200, 1e200}};
    EXPECT_NEAR(segment.length() / 1e200, std::sqrt(Scalar{3}), 1e-12);
    EXPECT_THROW((void)segment.squaredLength(), std::overflow_error);
    EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
}

TEST(Segment3, SquaredLengthCanRemainFiniteNearMaximum)
{
    const Segment3 segment{Point3{}, Point3{1e154, 0, 0}};
    EXPECT_NEAR(segment.squaredLength() / 1e308, 1, 1e-12);
}

TEST(Segment3, TinyLengthAvoidsIntermediateUnderflow)
{
    const Segment3 segment{Point3{}, Point3{1e-200, 1e-200, 1e-200}};
    EXPECT_NEAR(segment.length() / 1e-200, std::sqrt(Scalar{3}), 1e-12);
    EXPECT_DOUBLE_EQ(segment.squaredLength(), 0); // Rounded underflow is finite.
    EXPECT_TRUE(segment.isDegenerate());
}

TEST(Segment3, OverflowingDifferenceStillHasFiniteMidpointAndDirection)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        std::array<Scalar, 3> values{}, negatives{};
        values[axis] = maximum;
        negatives[axis] = -maximum;
        const Point3 a{negatives[0], negatives[1], negatives[2]}, b{values[0], values[1], values[2]};
        const Segment3 segment{a, b}, reverse{b, a};
        EXPECT_THROW((void)segment.length(), std::overflow_error);
        EXPECT_THROW((void)segment.squaredLength(), std::overflow_error);
        EXPECT_FALSE(segment.isDegenerate(maximum));
        EXPECT_TRUE(areCoincident(segment.midpoint(), Point3{}, 0));
        EXPECT_NEAR(segment.direction().length(), 1, 1e-12);
        EXPECT_TRUE(microsw::math::almostEqual(reverse.direction(), -segment.direction()));
    }
}

TEST(Segment3, OverflowingNormStillHasUnitDirection)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Segment3 segment{Point3{}, Point3{maximum, maximum, maximum}};
    EXPECT_THROW((void)segment.length(), std::overflow_error);
    EXPECT_THROW((void)segment.squaredLength(), std::overflow_error);
    const auto direction = segment.direction();
    EXPECT_NEAR(direction.x(), 1 / std::sqrt(Scalar{3}), 1e-12);
    EXPECT_NEAR(direction.y(), 1 / std::sqrt(Scalar{3}), 1e-12);
    EXPECT_NEAR(direction.z(), 1 / std::sqrt(Scalar{3}), 1e-12);
}

TEST(Segment3, OverflowScalingPreservesUnequalComponentRatios)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Segment3 segment{Point3{-maximum, 0, 0},
        Point3{maximum, maximum, -maximum}};
    const auto direction = segment.direction();
    EXPECT_NEAR(direction.y() / direction.x(), 0.5, 1e-12);
    EXPECT_NEAR(direction.z() / direction.x(), -0.5, 1e-12);
    EXPECT_NEAR(direction.length(), 1, 1e-12);
}

TEST(Segment3, MidpointHandlesSameSignExtremesAndSubnormals)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const auto tiny = std::numeric_limits<Scalar>::denorm_min();
    for (Scalar value : {maximum, -maximum, tiny, -tiny})
    {
        const Point3 endpoint{value, value, value};
        EXPECT_TRUE(areCoincident((Segment3{endpoint, endpoint}).midpoint(), endpoint, 0));
    }
    const Point3 a{maximum, maximum, maximum}, b{maximum / 2, maximum / 2, maximum / 2};
    EXPECT_TRUE(areCoincident((Segment3{a, b}).midpoint(), Point3{maximum * 0.75, maximum * 0.75, maximum * 0.75}, 0));
    EXPECT_TRUE(areCoincident((Segment3{b, a}).midpoint(), (Segment3{a, b}).midpoint(), 0));
}

}
