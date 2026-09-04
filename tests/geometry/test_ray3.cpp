#include "core/geometry/Ray3.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/Line3.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <utility>

namespace
{
using microsw::geometry::Ray3;
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::areCoincident;
using microsw::math::Vector3;
using microsw::math::Scalar;
using microsw::math::almostEqual;

template<class T> concept HasEquality = requires(T value) { value == value; };
static_assert(!std::is_default_constructible_v<Ray3>);
static_assert(!std::is_base_of_v<Point3, Ray3> && !std::is_base_of_v<Vector3, Ray3>);
static_assert(!HasEquality<Ray3>);
static_assert(!std::is_base_of_v<Line3, Ray3>);
static_assert(!std::is_constructible_v<Ray3, Line3>);
static_assert(!std::is_constructible_v<Line3, Ray3>);
static_assert(!std::is_constructible_v<Ray3, Point3, Point3>);
static_assert(!std::is_constructible_v<Ray3, Segment3>);
static_assert(std::is_same_v<decltype(std::declval<Ray3&>().origin()), const Point3&>);
static_assert(std::is_same_v<decltype(std::declval<Ray3&>().direction()), const Vector3&>);
static_assert(noexcept(std::declval<const Ray3&>().origin()));
static_assert(noexcept(std::declval<const Ray3&>().direction()));
static_assert(std::is_copy_constructible_v<Ray3> && std::is_copy_assignable_v<Ray3>);
static_assert(std::is_move_constructible_v<Ray3> && std::is_move_assignable_v<Ray3>);

TEST(Ray3, EveryAxisPreservesPositiveAndNegativeOrientation)
{
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 3> values{};
            values[axis] = sign * 4;
            const Vector3 input{values[0], values[1], values[2]};
            const Ray3 line{Point3{}, input};
            EXPECT_TRUE(almostEqual(line.direction(), input * 0.25));
            EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
        }
}

TEST(Ray3, OppositeDirectionsPreserveDistinctHalfLines)
{
    const Vector3 input{2, -3, 6};
    const Ray3 line{Point3{}, input}, reversed{Point3{}, -input};
    EXPECT_TRUE(almostEqual(line.direction(), Vector3{2.0 / 7, -3.0 / 7, 6.0 / 7}));
    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
    EXPECT_TRUE(almostEqual(reversed.direction(), -line.direction()));
    // Opposite rays share an origin, but retain distinct oriented domains.
    EXPECT_FALSE(almostEqual(reversed.direction(), line.direction()));
}

TEST(Ray3, PositiveScaleDoesNotChangeStoredUnitDirection)
{
    const Vector3 input{2, -3, 6};
    const Ray3 reference{Point3{}, input};
    for (Scalar scale : {1e-8, 0.5, 2.0, 1e200})
    {
        const Ray3 line{Point3{}, input * scale};
        EXPECT_TRUE(almostEqual(line.direction(), reference.direction()));
        EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
    }
}

TEST(Ray3, OriginIsNotCanonicalizedAndInputsRemainUnchanged)
{
    const Point3 origin{2, -3, 6};
    const Vector3 input{2, -3, 6};
    const Ray3 line{origin, input};
    EXPECT_EQ(line.origin().x(), origin.x());
    EXPECT_EQ(line.origin().y(), origin.y());
    EXPECT_EQ(line.origin().z(), origin.z());
    EXPECT_TRUE(almostEqual(input, Vector3{2, -3, 6}));
    const Ray3 shifted{origin + input, input};
    EXPECT_TRUE(areCoincident(shifted.origin(), origin + input, 0));
    EXPECT_FALSE(areCoincident(shifted.origin(), line.origin(), 0));
}

TEST(Ray3, ExtremeFiniteOriginsArePreservedExactly)
{
    for (Scalar value : {std::numeric_limits<Scalar>::max(),
                        std::numeric_limits<Scalar>::lowest(),
                        std::numeric_limits<Scalar>::denorm_min(), -0.0})
    {
        const Point3 origin{value, value, value};
        const Ray3 line{origin, Vector3{2, -3, 6}};
        EXPECT_EQ(line.origin().x(), value);
        EXPECT_EQ(std::signbit(line.origin().x()), std::signbit(value));
        EXPECT_EQ(line.origin().y(), value);
        EXPECT_EQ(std::signbit(line.origin().y()), std::signbit(value));
        EXPECT_EQ(line.origin().z(), value);
        EXPECT_EQ(std::signbit(line.origin().z()), std::signbit(value));
    }
}

TEST(Ray3, RejectsNonFiniteComponentsInEveryAxis)
{
    for (Scalar value : {std::numeric_limits<Scalar>::quiet_NaN(),
                         std::numeric_limits<Scalar>::infinity(),
                         -std::numeric_limits<Scalar>::infinity()})
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
            std::array<Scalar, 3> values{};
            values.fill(1);
            values[axis] = value;
            const Vector3 input{values[0], values[1], values[2]};
            EXPECT_THROW((void)Ray3(Point3{}, input), std::invalid_argument);
        }
}

TEST(Ray3, RejectsZeroSignedZeroAndSubnormalDirections)
{
    for (Scalar value : {0.0, -0.0, std::numeric_limits<Scalar>::denorm_min()})
        EXPECT_THROW((void)Ray3(Point3{}, Vector3{value, value, value}),
            std::invalid_argument);
}

TEST(Ray3, GeometricBoundaryIsInclusiveOnEveryAxis)
{
    const auto above = std::nextafter(1e-9, std::numeric_limits<Scalar>::infinity());
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar sign : {-1.0, 1.0})
            for (Scalar magnitude : {5e-10, 1e-9, above})
            {
                std::array<Scalar, 3> values{};
                values[axis] = sign * magnitude;
                const Vector3 input{values[0], values[1], values[2]};
                if (magnitude <= 1e-9)
                    EXPECT_THROW((void)Ray3(Point3{}, input), std::invalid_argument);
                else
                {
                    const Ray3 line{Point3{}, input};
                    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
                    EXPECT_TRUE(almostEqual(line.direction(), input * (1 / magnitude)));
                }
            }
}

TEST(Ray3, GeometricThresholdIsNotNumericZero)
{
    const Vector3 input{5e-10, 0, 0};
    EXPECT_FALSE(microsw::math::isNearlyZero(input.length()));
    EXPECT_NO_THROW((void)input.normalized());
    EXPECT_THROW((void)Ray3(Point3{}, input), std::invalid_argument);
}

TEST(Ray3, ThresholdUsesEuclideanMagnitudeNotComponentMaximum)
{
    EXPECT_THROW((void)Ray3(Point3{}, Vector3{0.5e-9, 0.5e-9, 0.5e-9}),
        std::invalid_argument);
    const Ray3 line{Point3{}, Vector3{0.8e-9, 0.8e-9, 0.8e-9}};
    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
}

TEST(Ray3, MaximumFiniteComponentsNormalizeWithoutNormOverflow)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (Scalar sign : {-1.0, 1.0})
    {
        const Ray3 line{Point3{}, Vector3{sign * maximum, sign * maximum, sign * maximum}};
        EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
        EXPECT_TRUE(std::isfinite(line.direction().x()));
        EXPECT_TRUE(almostEqual(line.direction().x(), sign / std::sqrt(Scalar{3})));
        EXPECT_TRUE(std::isfinite(line.direction().y()));
        EXPECT_TRUE(almostEqual(line.direction().y(), sign / std::sqrt(Scalar{3})));
        EXPECT_TRUE(std::isfinite(line.direction().z()));
        EXPECT_TRUE(almostEqual(line.direction().z(), sign / std::sqrt(Scalar{3})));
    }
}

TEST(Ray3, UnequalExtremeComponentsPreserveRatios)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Ray3 line{Point3{}, Vector3{maximum, -maximum / 2, maximum / 4}};
    EXPECT_TRUE(almostEqual(line.direction().y() / line.direction().x(), -0.5));
    EXPECT_TRUE(almostEqual(line.direction().z() / line.direction().x(), 0.25));
    EXPECT_TRUE(almostEqual(line.direction().length(), 1.0));
}

TEST(Ray3, CopyMoveAndAssignmentPreserveIndependentValues)
{
    Point3 origin{2, -3, 6};
    Vector3 input{2, -3, 6};
    const Ray3 original{origin, input};
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
    origin = Point3{};
    input = Vector3{};
    EXPECT_TRUE(areCoincident(original.origin(), Point3{2, -3, 6}, 0));
    EXPECT_TRUE(almostEqual(original.direction().length(), 1.0));
}

}
