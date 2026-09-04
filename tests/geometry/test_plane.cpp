#include "core/geometry/Plane.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/Line3.h"
#include "core/geometry/Ray3.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <utility>

namespace
{
using microsw::geometry::Plane;
using microsw::geometry::Line3;
using microsw::geometry::Ray3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::areCoincident;
using microsw::math::Vector3;
using microsw::math::Scalar;
using microsw::math::almostEqual;

template<class T> concept HasEquality = requires(T value) { value == value; };
static_assert(!std::is_default_constructible_v<Plane>);
static_assert(!std::is_base_of_v<Point3, Plane> && !std::is_base_of_v<Vector3, Plane>);
static_assert(!HasEquality<Plane>);
static_assert(!std::is_base_of_v<Ray3, Plane>);
static_assert(!std::is_constructible_v<Plane, Point3, Point3, Point3>);
static_assert(!std::is_constructible_v<Plane, Point3, Vector3, Vector3>);
static_assert(!std::is_constructible_v<Plane, Scalar, Scalar, Scalar, Scalar>);
static_assert(!std::is_base_of_v<Line3, Plane>);
static_assert(!std::is_constructible_v<Plane, Line3>);
static_assert(!std::is_constructible_v<Line3, Plane>);
static_assert(!std::is_constructible_v<Plane, Point3, Point3>);
static_assert(!std::is_constructible_v<Plane, Segment3>);
static_assert(std::is_same_v<decltype(std::declval<Plane&>().origin()), const Point3&>);
static_assert(std::is_same_v<decltype(std::declval<Plane&>().normal()), const Vector3&>);
static_assert(noexcept(std::declval<const Plane&>().origin()));
static_assert(noexcept(std::declval<const Plane&>().normal()));
static_assert(std::is_copy_constructible_v<Plane> && std::is_copy_assignable_v<Plane>);
static_assert(std::is_move_constructible_v<Plane> && std::is_move_assignable_v<Plane>);

TEST(Plane, EveryAxisPreservesPositiveAndNegativeOrientation)
{
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 3> values{};
            values[axis] = sign * 4;
            const Vector3 input{values[0], values[1], values[2]};
            const Plane line{Point3{}, input};
            EXPECT_TRUE(almostEqual(line.normal(), input * 0.25));
            EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
        }
}

TEST(Plane, OppositeNormalsPreserveOrientation)
{
    const Vector3 input{2, -3, 6};
    const Plane line{Point3{}, input}, reversed{Point3{}, -input};
    EXPECT_TRUE(almostEqual(line.normal(), Vector3{2.0 / 7, -3.0 / 7, 6.0 / 7}));
    EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
    EXPECT_TRUE(almostEqual(reversed.normal(), -line.normal()));
    // Opposite normals encode orientation; no geometric equivalence query exists.
    EXPECT_FALSE(almostEqual(reversed.normal(), line.normal()));
}

TEST(Plane, PositiveScaleDoesNotChangeStoredUnitNormal)
{
    const Vector3 input{2, -3, 6};
    const Plane reference{Point3{}, input};
    for (Scalar scale : {1e-8, 0.5, 2.0, 1e200})
    {
        const Plane line{Point3{}, input * scale};
        EXPECT_TRUE(almostEqual(line.normal(), reference.normal()));
        EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
    }
}

TEST(Plane, OriginIsNotCanonicalizedAndInputsRemainUnchanged)
{
    const Point3 origin{2, -3, 6};
    const Vector3 input{2, -3, 6};
    const Plane line{origin, input};
    EXPECT_EQ(line.origin().x(), origin.x());
    EXPECT_EQ(line.origin().y(), origin.y());
    EXPECT_EQ(line.origin().z(), origin.z());
    EXPECT_TRUE(almostEqual(input, Vector3{2, -3, 6}));
    const Plane shifted{origin + input, input};
    EXPECT_TRUE(areCoincident(shifted.origin(), origin + input, 0));
    EXPECT_FALSE(areCoincident(shifted.origin(), line.origin(), 0));
}

TEST(Plane, ExtremeFiniteOriginsArePreservedExactly)
{
    for (Scalar value : {std::numeric_limits<Scalar>::max(),
                        std::numeric_limits<Scalar>::lowest(),
                        std::numeric_limits<Scalar>::denorm_min(), -0.0})
    {
        const Point3 origin{value, value, value};
        const Plane line{origin, Vector3{2, -3, 6}};
        EXPECT_EQ(line.origin().x(), value);
        EXPECT_EQ(std::signbit(line.origin().x()), std::signbit(value));
        EXPECT_EQ(line.origin().y(), value);
        EXPECT_EQ(std::signbit(line.origin().y()), std::signbit(value));
        EXPECT_EQ(line.origin().z(), value);
        EXPECT_EQ(std::signbit(line.origin().z()), std::signbit(value));
    }
}

TEST(Plane, RejectsNonFiniteComponentsInEveryAxis)
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
            EXPECT_THROW((void)Plane(Point3{}, input), std::invalid_argument);
        }
}

TEST(Plane, RejectsZeroSignedZeroAndSubnormalNormals)
{
    for (Scalar value : {0.0, -0.0, std::numeric_limits<Scalar>::denorm_min()})
        EXPECT_THROW((void)Plane(Point3{}, Vector3{value, value, value}),
            std::invalid_argument);
}

TEST(Plane, GeometricBoundaryIsInclusiveOnEveryAxis)
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
                    EXPECT_THROW((void)Plane(Point3{}, input), std::invalid_argument);
                else
                {
                    const Plane line{Point3{}, input};
                    EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
                    EXPECT_TRUE(almostEqual(line.normal(), input * (1 / magnitude)));
                }
            }
}

TEST(Plane, GeometricThresholdIsNotNumericZero)
{
    const Vector3 input{5e-10, 0, 0};
    EXPECT_FALSE(microsw::math::isNearlyZero(input.length()));
    EXPECT_NO_THROW((void)input.normalized());
    EXPECT_THROW((void)Plane(Point3{}, input), std::invalid_argument);
}

TEST(Plane, ThresholdUsesEuclideanMagnitudeNotComponentMaximum)
{
    EXPECT_THROW((void)Plane(Point3{}, Vector3{0.5e-9, 0.5e-9, 0}),
        std::invalid_argument);
    const Plane line{Point3{}, Vector3{0.8e-9, 0.8e-9, 0}};
    EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
}

TEST(Plane, MaximumFiniteComponentsNormalizeWithoutNormOverflow)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (Scalar sign : {-1.0, 1.0})
    {
        const Plane line{Point3{}, Vector3{sign * maximum, sign * maximum, sign * maximum}};
        EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
        EXPECT_TRUE(std::isfinite(line.normal().x()));
        EXPECT_TRUE(almostEqual(line.normal().x(), sign / std::sqrt(Scalar{3})));
        EXPECT_TRUE(std::isfinite(line.normal().y()));
        EXPECT_TRUE(almostEqual(line.normal().y(), sign / std::sqrt(Scalar{3})));
        EXPECT_TRUE(std::isfinite(line.normal().z()));
        EXPECT_TRUE(almostEqual(line.normal().z(), sign / std::sqrt(Scalar{3})));
    }
}

TEST(Plane, UnequalExtremeComponentsPreserveRatios)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Plane line{Point3{}, Vector3{maximum, -maximum / 2, maximum / 4}};
    EXPECT_TRUE(almostEqual(line.normal().y() / line.normal().x(), -0.5));
    EXPECT_TRUE(almostEqual(line.normal().z() / line.normal().x(), 0.25));
    EXPECT_TRUE(almostEqual(line.normal().length(), 1.0));
}

TEST(Plane, CopyMoveAndAssignmentPreserveIndependentValues)
{
    Point3 origin{2, -3, 6};
    Vector3 input{2, -3, 6};
    const Plane original{origin, input};
    auto copy = original;
    auto moved = std::move(copy);
    auto assigned = original;
    assigned = moved;
    auto moveAssigned = original;
    moveAssigned = std::move(assigned);
    EXPECT_TRUE(areCoincident(moved.origin(), original.origin(), 0));
    EXPECT_TRUE(almostEqual(moved.normal(), original.normal()));
    EXPECT_TRUE(areCoincident(moveAssigned.origin(), original.origin(), 0));
    EXPECT_TRUE(almostEqual(moveAssigned.normal(), original.normal()));
    origin = Point3{};
    input = Vector3{};
    EXPECT_TRUE(areCoincident(original.origin(), Point3{2, -3, 6}, 0));
    EXPECT_TRUE(almostEqual(original.normal().length(), 1.0));
}

}
