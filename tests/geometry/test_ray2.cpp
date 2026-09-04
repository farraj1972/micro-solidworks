#include "core/geometry/Ray2.h"
#include "core/geometry/Segment2.h"
#include "core/geometry/Line2.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <utility>

namespace
{
using microsw::geometry::Ray2;
using microsw::geometry::Line2;
using microsw::geometry::Point2;
using microsw::geometry::Segment2;
using microsw::geometry::areCoincident;
using microsw::math::Vector2;
using microsw::math::Scalar;
using microsw::math::almostEqual;

template<class T> concept HasEquality = requires(T value) { value == value; };
static_assert(!std::is_default_constructible_v<Ray2>);
static_assert(!std::is_base_of_v<Point2, Ray2> && !std::is_base_of_v<Vector2, Ray2>);
static_assert(!HasEquality<Ray2>);
static_assert(!std::is_base_of_v<Line2, Ray2>);
static_assert(!std::is_constructible_v<Ray2, Line2>);
static_assert(!std::is_constructible_v<Line2, Ray2>);
static_assert(!std::is_constructible_v<Ray2, Point2, Point2>);
static_assert(!std::is_constructible_v<Ray2, Segment2>);
static_assert(std::is_same_v<decltype(std::declval<Ray2&>().origin()), const Point2&>);
static_assert(std::is_same_v<decltype(std::declval<Ray2&>().direction()), const Vector2&>);
static_assert(noexcept(std::declval<const Ray2&>().origin()));
static_assert(noexcept(std::declval<const Ray2&>().direction()));
static_assert(std::is_copy_constructible_v<Ray2> && std::is_copy_assignable_v<Ray2>);
static_assert(std::is_move_constructible_v<Ray2> && std::is_move_assignable_v<Ray2>);

TEST(Ray2, EveryAxisPreservesPositiveAndNegativeOrientation)
{
    for (std::size_t axis = 0; axis < 2; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 2> values{};
            values[axis] = sign * 4;
            const Vector2 input{values[0], values[1]};
            const Ray2 line{Point2{}, input};
            EXPECT_TRUE(almostEqual(line.direction(), input * 0.25));
            EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
        }
}

TEST(Ray2, OppositeDirectionsPreserveDistinctHalfLines)
{
    const Vector2 input{3, -4};
    const Ray2 line{Point2{}, input}, reversed{Point2{}, -input};
    EXPECT_TRUE(almostEqual(line.direction(), Vector2{3.0 / 5, -4.0 / 5}));
    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
    EXPECT_TRUE(almostEqual(reversed.direction(), -line.direction()));
    // Opposite rays share an origin, but retain distinct oriented domains.
    EXPECT_FALSE(almostEqual(reversed.direction(), line.direction()));
}

TEST(Ray2, PositiveScaleDoesNotChangeStoredUnitDirection)
{
    const Vector2 input{3, -4};
    const Ray2 reference{Point2{}, input};
    for (Scalar scale : {1e-8, 0.5, 2.0, 1e200})
    {
        const Ray2 line{Point2{}, input * scale};
        EXPECT_TRUE(almostEqual(line.direction(), reference.direction()));
        EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
    }
}

TEST(Ray2, OriginIsNotCanonicalizedAndInputsRemainUnchanged)
{
    const Point2 origin{3, -4};
    const Vector2 input{3, -4};
    const Ray2 line{origin, input};
    EXPECT_EQ(line.origin().x(), origin.x());
    EXPECT_EQ(line.origin().y(), origin.y());
    EXPECT_TRUE(almostEqual(input, Vector2{3, -4}));
    const Ray2 shifted{origin + input, input};
    EXPECT_TRUE(areCoincident(shifted.origin(), origin + input, 0));
    EXPECT_FALSE(areCoincident(shifted.origin(), line.origin(), 0));
}

TEST(Ray2, ExtremeFiniteOriginsArePreservedExactly)
{
    for (Scalar value : {std::numeric_limits<Scalar>::max(),
                        std::numeric_limits<Scalar>::lowest(),
                        std::numeric_limits<Scalar>::denorm_min(), -0.0})
    {
        const Point2 origin{value, value};
        const Ray2 line{origin, Vector2{3, -4}};
        EXPECT_EQ(line.origin().x(), value);
        EXPECT_EQ(std::signbit(line.origin().x()), std::signbit(value));
        EXPECT_EQ(line.origin().y(), value);
        EXPECT_EQ(std::signbit(line.origin().y()), std::signbit(value));
    }
}

TEST(Ray2, RejectsNonFiniteComponentsInEveryAxis)
{
    for (Scalar value : {std::numeric_limits<Scalar>::quiet_NaN(),
                         std::numeric_limits<Scalar>::infinity(),
                         -std::numeric_limits<Scalar>::infinity()})
        for (std::size_t axis = 0; axis < 2; ++axis)
        {
            std::array<Scalar, 2> values{};
            values.fill(1);
            values[axis] = value;
            const Vector2 input{values[0], values[1]};
            EXPECT_THROW((void)Ray2(Point2{}, input), std::invalid_argument);
        }
}

TEST(Ray2, RejectsZeroSignedZeroAndSubnormalDirections)
{
    for (Scalar value : {0.0, -0.0, std::numeric_limits<Scalar>::denorm_min()})
        EXPECT_THROW((void)Ray2(Point2{}, Vector2{value, value}),
            std::invalid_argument);
}

TEST(Ray2, GeometricBoundaryIsInclusiveOnEveryAxis)
{
    const auto above = std::nextafter(1e-9, std::numeric_limits<Scalar>::infinity());
    for (std::size_t axis = 0; axis < 2; ++axis)
        for (Scalar sign : {-1.0, 1.0})
            for (Scalar magnitude : {5e-10, 1e-9, above})
            {
                std::array<Scalar, 2> values{};
                values[axis] = sign * magnitude;
                const Vector2 input{values[0], values[1]};
                if (magnitude <= 1e-9)
                    EXPECT_THROW((void)Ray2(Point2{}, input), std::invalid_argument);
                else
                {
                    const Ray2 line{Point2{}, input};
                    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
                    EXPECT_TRUE(almostEqual(line.direction(), input * (1 / magnitude)));
                }
            }
}

TEST(Ray2, GeometricThresholdIsNotNumericZero)
{
    const Vector2 input{5e-10, 0};
    EXPECT_FALSE(microsw::math::isNearlyZero(input.length()));
    EXPECT_NO_THROW((void)input.normalized());
    EXPECT_THROW((void)Ray2(Point2{}, input), std::invalid_argument);
}

TEST(Ray2, ThresholdUsesEuclideanMagnitudeNotComponentMaximum)
{
    EXPECT_THROW((void)Ray2(Point2{}, Vector2{0.5e-9, 0.5e-9}),
        std::invalid_argument);
    const Ray2 line{Point2{}, Vector2{0.8e-9, 0.8e-9}};
    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
}

TEST(Ray2, MaximumFiniteComponentsNormalizeWithoutNormOverflow)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (Scalar sign : {-1.0, 1.0})
    {
        const Ray2 line{Point2{}, Vector2{sign * maximum, sign * maximum}};
        EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
        EXPECT_TRUE(std::isfinite(line.direction().x()));
        EXPECT_TRUE(almostEqual(line.direction().x(), sign / std::sqrt(Scalar{2})));
        EXPECT_TRUE(std::isfinite(line.direction().y()));
        EXPECT_TRUE(almostEqual(line.direction().y(), sign / std::sqrt(Scalar{2})));
    }
}

TEST(Ray2, UnequalExtremeComponentsPreserveRatios)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Ray2 line{Point2{}, Vector2{maximum, -maximum / 2}};
    EXPECT_TRUE(almostEqual(line.direction().y() / line.direction().x(), -0.5));
    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
}

TEST(Ray2, CopyMoveAndAssignmentPreserveIndependentValues)
{
    Point2 origin{3, -4};
    Vector2 input{3, -4};
    const Ray2 original{origin, input};
    auto copy = original;
    auto moved = std::move(copy);
    auto assigned = original;
    assigned = moved;
    auto moveAssigned = original;
    moveAssigned = std::move(assigned);
    EXPECT_TRUE(areCoincident(moved.origin(), original.origin(), 0));
    EXPECT_TRUE(almostEqual(moved.direction(), original.direction()));
    EXPECT_TRUE(areCoincident(moveAssigned.origin(), original.origin(), 0));
    EXPECT_TRUE(almostEqual(moveAssigned.direction(), original.direction()));
    origin = Point2{};
    input = Vector2{};
    EXPECT_TRUE(areCoincident(original.origin(), Point2{3, -4}, 0));
    EXPECT_TRUE(almostEqual(original.direction().length(), 1.0));
}

}
