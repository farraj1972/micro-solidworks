#include "core/geometry/Point3.h"
#include "core/math/Tolerance.h"

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
using microsw::geometry::areCoincident;
using microsw::geometry::defaultGeometricTolerance;
using microsw::math::Scalar;
using microsw::math::Vector3;

template<class T> concept HasPointSum = requires(T value) { value + value; };
template<class T> concept HasEquality = requires(T value) { value == value; };
template<class T> concept HasInequality = requires(T value) { value != value; };
template<class T> concept HasScalarProduct = requires(T value) { value * Scalar{2}; };
template<class T> concept HasScalarQuotient = requires(T value) { value / Scalar{2}; };

static_assert(std::is_same_v<decltype(std::declval<Point3>() - std::declval<Point3>()), Vector3>);
static_assert(std::is_same_v<decltype(std::declval<Point3>() + std::declval<Vector3>()), Point3>);
static_assert(std::is_same_v<decltype(std::declval<Vector3>() + std::declval<Point3>()), Point3>);
static_assert(std::is_same_v<decltype(std::declval<Point3>() - std::declval<Vector3>()), Point3>);
static_assert(!std::is_base_of_v<Vector3, Point3>);
static_assert(!std::is_convertible_v<Point3, Vector3> && !std::is_convertible_v<Vector3, Point3>);
static_assert(!HasPointSum<Point3> && !HasEquality<Point3> && !HasInequality<Point3>);
static_assert(!HasScalarProduct<Point3> && !HasScalarQuotient<Point3>);
static_assert(std::is_copy_constructible_v<Point3> && std::is_move_constructible_v<Point3>);
static_assert(std::is_copy_assignable_v<Point3> && std::is_move_assignable_v<Point3>);
static_assert(std::is_same_v<decltype(std::declval<const Point3&>().x()), Scalar>);
static_assert(std::is_same_v<decltype(std::declval<const Point3&>().y()), Scalar>);
static_assert(std::is_same_v<decltype(std::declval<const Point3&>().z()), Scalar>);

TEST(Point3, DefaultOriginIsConstexpr)
{
    constexpr Point3 origin{};
    static_assert(origin.x() == 0.0 && noexcept(origin.x()));
    static_assert(origin.y() == 0.0 && noexcept(origin.y()));
    static_assert(origin.z() == 0.0 && noexcept(origin.z()));
    EXPECT_TRUE(areCoincident(origin, Point3{0, 0, 0}, 0.0));
}

TEST(Point3, ComponentsAndAccessorsPreservePosition)
{
    const Point3 point{1.25, -2.5, 4.75};
    EXPECT_DOUBLE_EQ(point.x(), 1.25);
    EXPECT_DOUBLE_EQ(point.y(), -2.5);
    EXPECT_DOUBLE_EQ(point.z(), 4.75);
}

TEST(Point3, AllFiniteCoordinateExtremesAreAccepted)
{
    for (Scalar value : {0.0, -0.0, std::numeric_limits<Scalar>::denorm_min(),
                         std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::max()})
    {
        const Point3 point{value, value, value};
        EXPECT_DOUBLE_EQ(point.x(), value);
        EXPECT_DOUBLE_EQ(point.y(), value);
        EXPECT_DOUBLE_EQ(point.z(), value);
    }
}

TEST(Point3, RejectsEveryNonFiniteCoordinate)
{
    for (Scalar value : {std::numeric_limits<Scalar>::quiet_NaN(),
                         std::numeric_limits<Scalar>::infinity(),
                         -std::numeric_limits<Scalar>::infinity()})
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
            std::array<Scalar, 3> coordinates{};
            coordinates[axis] = value;
            EXPECT_THROW((void)Point3(coordinates[0], coordinates[1], coordinates[2]), std::invalid_argument);
        }
}

TEST(Point3, PointDifferenceReturnsDisplacementWithCorrectSigns)
{
    const Point3 first{1, -2, 3}, second{4, 5, -6};
    const Vector3 difference = first - second;
    EXPECT_DOUBLE_EQ(difference.x(), -3);
    EXPECT_DOUBLE_EQ(difference.y(), -7);
    EXPECT_DOUBLE_EQ(difference.z(), 9);
    const auto reverse = second - first;
    EXPECT_DOUBLE_EQ(reverse.x(), -difference.x());
    EXPECT_DOUBLE_EQ(reverse.y(), -difference.y());
    EXPECT_DOUBLE_EQ(reverse.z(), -difference.z());
}

TEST(Point3, TranslationSupportsBothAdditionOrdersAndSubtraction)
{
    const Point3 point{1, -2, 3};
    const Vector3 displacement{4, 5, -6};
    EXPECT_TRUE(areCoincident(point + displacement, Point3{5, 3, -3}, 0.0));
    EXPECT_TRUE(areCoincident(displacement + point, Point3{5, 3, -3}, 0.0));
    EXPECT_TRUE(areCoincident(point - displacement, Point3{-3, -7, 9}, 0.0));
    EXPECT_TRUE(areCoincident((point + displacement) - displacement, point, 0.0));
}

TEST(Point3, ArithmeticAndCopyLeaveOperandsUnchanged)
{
    const Point3 original{2, -3, 6};
    auto copy = original;
    const Vector3 vector{1, 2, -4};
    copy = original + vector;
    (void)(original - copy);
    (void)(original - vector);
    EXPECT_TRUE(areCoincident(original, Point3{2, -3, 6}, 0.0));
    EXPECT_FALSE(areCoincident(copy, original));
    EXPECT_DOUBLE_EQ(vector.x(), 1);
    EXPECT_DOUBLE_EQ(vector.y(), 2);
    EXPECT_DOUBLE_EQ(vector.z(), -4);
}

TEST(Point3, ExactCoincidenceAndSignedZero)
{
    const Point3 point{2, -3, 6};
    EXPECT_TRUE(areCoincident(point, point));
    EXPECT_TRUE(areCoincident(Point3{-0.0, -0.0, -0.0}, Point3{}, 0.0));
}

TEST(Point3, DefaultToleranceAppliesInEveryAxisIncludingBoundary)
{
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 3> coordinates{};
            coordinates[axis] = sign * defaultGeometricTolerance * 0.5;
            EXPECT_TRUE(areCoincident(Point3{}, Point3{coordinates[0], coordinates[1], coordinates[2]}));
            coordinates[axis] = sign * defaultGeometricTolerance;
            EXPECT_TRUE(areCoincident(Point3{}, Point3{coordinates[0], coordinates[1], coordinates[2]}));
            coordinates[axis] = sign * defaultGeometricTolerance * 1.01;
            EXPECT_FALSE(areCoincident(Point3{}, Point3{coordinates[0], coordinates[1], coordinates[2]}));
        }
}

TEST(Point3, CoincidenceUsesEuclideanSphereNotComponentBounds)
{
    const auto t = defaultGeometricTolerance;
    const Point3 inside{0.5 * t, 0.5 * t, 0.5 * t};
    const Point3 outside{0.8 * t, 0.8 * t, 0.8 * t};
    EXPECT_TRUE(areCoincident(Point3{}, inside));
    EXPECT_FALSE(areCoincident(Point3{}, outside));
    EXPECT_EQ(areCoincident(inside, outside), areCoincident(outside, inside));
}

TEST(Point3, CustomToleranceIncludesEuclideanBoundary)
{
    const Point3 point{0, 3, 4};
    EXPECT_TRUE(areCoincident(Point3{}, point, 5.0));
    EXPECT_FALSE(areCoincident(Point3{}, point, 4.99));
    EXPECT_FALSE(areCoincident(Point3{}, point));
}

TEST(Point3, ZeroToleranceRequiresExactPositionEvenForTinyDifferences)
{
    const auto tiny = std::numeric_limits<Scalar>::denorm_min();
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        std::array<Scalar, 3> coordinates{};
        coordinates[axis] = tiny;
        EXPECT_FALSE(areCoincident(Point3{}, Point3{coordinates[0], coordinates[1], coordinates[2]}, 0.0));
        EXPECT_TRUE(areCoincident(Point3{}, Point3{coordinates[0], coordinates[1], coordinates[2]}, defaultGeometricTolerance));
    }
}

TEST(Point3, InvalidToleranceIsRejectedEvenForIdenticalPoints)
{
    const Point3 origin{};
    for (Scalar tolerance : {-1.0, -std::numeric_limits<Scalar>::denorm_min(),
                             std::numeric_limits<Scalar>::quiet_NaN(),
                             std::numeric_limits<Scalar>::infinity(),
                             -std::numeric_limits<Scalar>::infinity()})
        EXPECT_THROW((void)areCoincident(origin, origin, tolerance), std::invalid_argument);
}

TEST(Point3, GeometricAndNumericToleranceRemainDistinct)
{
    static_assert(defaultGeometricTolerance == 1.0e-9);
    static_assert(microsw::math::defaultAbsoluteTolerance == 1.0e-12);
    static_assert(microsw::math::defaultRelativeTolerance == 1.0e-12);
    // At the origin the relative rule cannot obscure the policy difference.
    EXPECT_FALSE(microsw::math::almostEqual(0.0, 5.0e-10));
    EXPECT_TRUE(areCoincident(Point3{}, Point3{5.0e-10, 0, 0}));
}

TEST(Point3, CoincidenceDoesNotUseRelativeToleranceAtLargePositions)
{
    EXPECT_TRUE(microsw::math::almostEqual(1.0e12, 1.0e12 + 0.5));
    EXPECT_FALSE(areCoincident(
        Point3{1.0e12, 0, 0}, Point3{1.0e12 + 0.5, 0, 0}));
}

TEST(Point3, LargeFiniteCoordinatesAndTolerancesAvoidSquareOverflow)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Point3 axisPoint{maximum, 0, 0};
    EXPECT_TRUE(areCoincident(axisPoint, axisPoint, 0.0));
    EXPECT_TRUE(areCoincident(Point3{}, axisPoint, maximum));
    EXPECT_FALSE(areCoincident(Point3{}, axisPoint, maximum / 2));
    const Point3 diagonal{maximum / 2, maximum / 2, maximum / 2};
    EXPECT_TRUE(areCoincident(Point3{}, diagonal, maximum));
    EXPECT_FALSE(areCoincident(Point3{}, diagonal, maximum / 2));
    const Point3 unrepresentableNorm{maximum, maximum, maximum};
    EXPECT_FALSE(areCoincident(Point3{}, unrepresentableNorm, maximum));
    const auto adjacent = std::nextafter(maximum, 0.0);
    const auto gap = maximum - adjacent;
    const Point3 neighbor{adjacent, 0, 0};
    EXPECT_TRUE(areCoincident(axisPoint, neighbor, gap));
    EXPECT_FALSE(areCoincident(axisPoint, neighbor, gap / 2));
}

TEST(Point3, UnrepresentableCoordinateDifferencesAreNotCoincident)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        std::array<Scalar, 3> positive{}, negative{};
        positive[axis] = maximum;
        negative[axis] = -maximum;
        const Point3 a{positive[0], positive[1], positive[2]}, b{negative[0], negative[1], negative[2]};
        EXPECT_FALSE(areCoincident(a, b, maximum));
        EXPECT_FALSE(areCoincident(b, a, maximum));
    }
}

TEST(Point3, TinyEuclideanDisplacementsAvoidSquareUnderflow)
{
    const Point3 diagonal{1.0e-200, 1.0e-200, 1.0e-200};
    EXPECT_FALSE(areCoincident(Point3{}, diagonal, 1.1e-200));
    EXPECT_TRUE(areCoincident(Point3{}, diagonal, 2.0e-200));
}

TEST(Point3, ArithmeticOverflowIsRejectedForEveryCoordinateAndSign)
{
    for (std::size_t axis = 0; axis < 3; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 3> values{}, opposite{};
            values[axis] = sign * std::numeric_limits<Scalar>::max();
            opposite[axis] = -values[axis];
            const Point3 point{values[0], values[1], values[2]}, other{opposite[0], opposite[1], opposite[2]};
            const Vector3 vector{values[0], values[1], values[2]}, negative{opposite[0], opposite[1], opposite[2]};
            EXPECT_THROW((void)(point + vector), std::overflow_error);
            EXPECT_THROW((void)(vector + point), std::overflow_error);
            EXPECT_THROW((void)(point - negative), std::overflow_error);
            EXPECT_THROW((void)(point - other), std::overflow_error);
            EXPECT_TRUE(areCoincident(point, Point3{values[0], values[1], values[2]}, 0.0));
        }
}

TEST(Point3, NonFiniteVectorInputsAreInvalidArguments)
{
    for (Scalar value : {std::numeric_limits<Scalar>::quiet_NaN(),
                         std::numeric_limits<Scalar>::infinity(),
                         -std::numeric_limits<Scalar>::infinity()})
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
            std::array<Scalar, 3> values{};
            values[axis] = value;
            const Vector3 vector{values[0], values[1], values[2]};
            EXPECT_THROW((void)(Point3{} + vector), std::invalid_argument);
            EXPECT_THROW((void)(vector + Point3{}), std::invalid_argument);
            EXPECT_THROW((void)(Point3{} - vector), std::invalid_argument);
        }
}

}
