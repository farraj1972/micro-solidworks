#include "core/geometry/Point2.h"
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
using microsw::geometry::Point2;
using microsw::geometry::areCoincident;
using microsw::geometry::defaultGeometricTolerance;
using microsw::math::Scalar;
using microsw::math::Vector2;

template<class T> concept HasPointSum = requires(T value) { value + value; };
template<class T> concept HasEquality = requires(T value) { value == value; };
template<class T> concept HasInequality = requires(T value) { value != value; };
template<class T> concept HasScalarProduct = requires(T value) { value * Scalar{2}; };
template<class T> concept HasScalarQuotient = requires(T value) { value / Scalar{2}; };

static_assert(std::is_same_v<decltype(std::declval<Point2>() - std::declval<Point2>()), Vector2>);
static_assert(std::is_same_v<decltype(std::declval<Point2>() + std::declval<Vector2>()), Point2>);
static_assert(std::is_same_v<decltype(std::declval<Vector2>() + std::declval<Point2>()), Point2>);
static_assert(std::is_same_v<decltype(std::declval<Point2>() - std::declval<Vector2>()), Point2>);
static_assert(!std::is_base_of_v<Vector2, Point2>);
static_assert(!std::is_convertible_v<Point2, Vector2> && !std::is_convertible_v<Vector2, Point2>);
static_assert(!HasPointSum<Point2> && !HasEquality<Point2> && !HasInequality<Point2>);
static_assert(!HasScalarProduct<Point2> && !HasScalarQuotient<Point2>);
static_assert(std::is_copy_constructible_v<Point2> && std::is_move_constructible_v<Point2>);
static_assert(std::is_copy_assignable_v<Point2> && std::is_move_assignable_v<Point2>);
static_assert(std::is_same_v<decltype(std::declval<const Point2&>().x()), Scalar>);
static_assert(std::is_same_v<decltype(std::declval<const Point2&>().y()), Scalar>);

TEST(Point2, DefaultOriginIsConstexpr)
{
    constexpr Point2 origin{};
    static_assert(origin.x() == 0.0 && noexcept(origin.x()));
    static_assert(origin.y() == 0.0 && noexcept(origin.y()));
    EXPECT_TRUE(areCoincident(origin, Point2{0, 0}, 0.0));
}

TEST(Point2, ComponentsAndAccessorsPreservePosition)
{
    const Point2 point{1.25, -2.5};
    EXPECT_DOUBLE_EQ(point.x(), 1.25);
    EXPECT_DOUBLE_EQ(point.y(), -2.5);
}

TEST(Point2, AllFiniteCoordinateExtremesAreAccepted)
{
    for (Scalar value : {0.0, -0.0, std::numeric_limits<Scalar>::denorm_min(),
                         std::numeric_limits<Scalar>::lowest(), std::numeric_limits<Scalar>::max()})
    {
        const Point2 point{value, value};
        EXPECT_DOUBLE_EQ(point.x(), value);
        EXPECT_DOUBLE_EQ(point.y(), value);
    }
}

TEST(Point2, RejectsEveryNonFiniteCoordinate)
{
    for (Scalar value : {std::numeric_limits<Scalar>::quiet_NaN(),
                         std::numeric_limits<Scalar>::infinity(),
                         -std::numeric_limits<Scalar>::infinity()})
        for (std::size_t axis = 0; axis < 2; ++axis)
        {
            std::array<Scalar, 2> coordinates{};
            coordinates[axis] = value;
            EXPECT_THROW((void)Point2(coordinates[0], coordinates[1]), std::invalid_argument);
        }
}

TEST(Point2, PointDifferenceReturnsDisplacementWithCorrectSigns)
{
    const Point2 first{1, -2}, second{4, 5};
    const Vector2 difference = first - second;
    EXPECT_DOUBLE_EQ(difference.x(), -3);
    EXPECT_DOUBLE_EQ(difference.y(), -7);
    const auto reverse = second - first;
    EXPECT_DOUBLE_EQ(reverse.x(), -difference.x());
    EXPECT_DOUBLE_EQ(reverse.y(), -difference.y());
}

TEST(Point2, TranslationSupportsBothAdditionOrdersAndSubtraction)
{
    const Point2 point{1, -2};
    const Vector2 displacement{4, 5};
    EXPECT_TRUE(areCoincident(point + displacement, Point2{5, 3}, 0.0));
    EXPECT_TRUE(areCoincident(displacement + point, Point2{5, 3}, 0.0));
    EXPECT_TRUE(areCoincident(point - displacement, Point2{-3, -7}, 0.0));
    EXPECT_TRUE(areCoincident((point + displacement) - displacement, point, 0.0));
}

TEST(Point2, ArithmeticAndCopyLeaveOperandsUnchanged)
{
    const Point2 original{2, -3};
    auto copy = original;
    const Vector2 vector{1, 2};
    copy = original + vector;
    (void)(original - copy);
    (void)(original - vector);
    EXPECT_TRUE(areCoincident(original, Point2{2, -3}, 0.0));
    EXPECT_FALSE(areCoincident(copy, original));
    EXPECT_DOUBLE_EQ(vector.x(), 1);
    EXPECT_DOUBLE_EQ(vector.y(), 2);
}

TEST(Point2, ExactCoincidenceAndSignedZero)
{
    const Point2 point{2, -3};
    EXPECT_TRUE(areCoincident(point, point));
    EXPECT_TRUE(areCoincident(Point2{-0.0, -0.0}, Point2{}, 0.0));
}

TEST(Point2, DefaultToleranceAppliesInEveryAxisIncludingBoundary)
{
    for (std::size_t axis = 0; axis < 2; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 2> coordinates{};
            coordinates[axis] = sign * defaultGeometricTolerance * 0.5;
            EXPECT_TRUE(areCoincident(Point2{}, Point2{coordinates[0], coordinates[1]}));
            coordinates[axis] = sign * defaultGeometricTolerance;
            EXPECT_TRUE(areCoincident(Point2{}, Point2{coordinates[0], coordinates[1]}));
            coordinates[axis] = sign * defaultGeometricTolerance * 1.01;
            EXPECT_FALSE(areCoincident(Point2{}, Point2{coordinates[0], coordinates[1]}));
        }
}

TEST(Point2, CoincidenceUsesEuclideanDiskNotComponentBounds)
{
    const auto t = defaultGeometricTolerance;
    const Point2 inside{0.5 * t, 0.5 * t};
    const Point2 outside{0.8 * t, 0.8 * t};
    EXPECT_TRUE(areCoincident(Point2{}, inside));
    EXPECT_FALSE(areCoincident(Point2{}, outside));
    EXPECT_EQ(areCoincident(inside, outside), areCoincident(outside, inside));
}

TEST(Point2, CustomToleranceIncludesEuclideanBoundary)
{
    const Point2 point{3, 4};
    EXPECT_TRUE(areCoincident(Point2{}, point, 5.0));
    EXPECT_FALSE(areCoincident(Point2{}, point, 4.99));
    EXPECT_FALSE(areCoincident(Point2{}, point));
}

TEST(Point2, ZeroToleranceRequiresExactPositionEvenForTinyDifferences)
{
    const auto tiny = std::numeric_limits<Scalar>::denorm_min();
    for (std::size_t axis = 0; axis < 2; ++axis)
    {
        std::array<Scalar, 2> coordinates{};
        coordinates[axis] = tiny;
        EXPECT_FALSE(areCoincident(Point2{}, Point2{coordinates[0], coordinates[1]}, 0.0));
        EXPECT_TRUE(areCoincident(Point2{}, Point2{coordinates[0], coordinates[1]}, defaultGeometricTolerance));
    }
}

TEST(Point2, InvalidToleranceIsRejectedEvenForIdenticalPoints)
{
    const Point2 origin{};
    for (Scalar tolerance : {-1.0, -std::numeric_limits<Scalar>::denorm_min(),
                             std::numeric_limits<Scalar>::quiet_NaN(),
                             std::numeric_limits<Scalar>::infinity(),
                             -std::numeric_limits<Scalar>::infinity()})
        EXPECT_THROW((void)areCoincident(origin, origin, tolerance), std::invalid_argument);
}

TEST(Point2, GeometricAndNumericToleranceRemainDistinct)
{
    static_assert(defaultGeometricTolerance == 1.0e-9);
    static_assert(microsw::math::defaultAbsoluteTolerance == 1.0e-12);
    static_assert(microsw::math::defaultRelativeTolerance == 1.0e-12);
    // At the origin the relative rule cannot obscure the policy difference.
    EXPECT_FALSE(microsw::math::almostEqual(0.0, 5.0e-10));
    EXPECT_TRUE(areCoincident(Point2{}, Point2{5.0e-10, 0}));
}

TEST(Point2, CoincidenceDoesNotUseRelativeToleranceAtLargePositions)
{
    EXPECT_TRUE(microsw::math::almostEqual(1.0e12, 1.0e12 + 0.5));
    EXPECT_FALSE(areCoincident(
        Point2{1.0e12, 0}, Point2{1.0e12 + 0.5, 0}));
}

TEST(Point2, LargeFiniteCoordinatesAndTolerancesAvoidSquareOverflow)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Point2 axisPoint{maximum, 0};
    EXPECT_TRUE(areCoincident(axisPoint, axisPoint, 0.0));
    EXPECT_TRUE(areCoincident(Point2{}, axisPoint, maximum));
    EXPECT_FALSE(areCoincident(Point2{}, axisPoint, maximum / 2));
    const Point2 diagonal{maximum / 2, maximum / 2};
    EXPECT_TRUE(areCoincident(Point2{}, diagonal, maximum));
    EXPECT_FALSE(areCoincident(Point2{}, diagonal, maximum / 2));
    const Point2 unrepresentableNorm{maximum, maximum};
    EXPECT_FALSE(areCoincident(Point2{}, unrepresentableNorm, maximum));
    const auto adjacent = std::nextafter(maximum, 0.0);
    const auto gap = maximum - adjacent;
    const Point2 neighbor{adjacent, 0};
    EXPECT_TRUE(areCoincident(axisPoint, neighbor, gap));
    EXPECT_FALSE(areCoincident(axisPoint, neighbor, gap / 2));
}

TEST(Point2, UnrepresentableCoordinateDifferencesAreNotCoincident)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    for (std::size_t axis = 0; axis < 2; ++axis)
    {
        std::array<Scalar, 2> positive{}, negative{};
        positive[axis] = maximum;
        negative[axis] = -maximum;
        const Point2 a{positive[0], positive[1]}, b{negative[0], negative[1]};
        EXPECT_FALSE(areCoincident(a, b, maximum));
        EXPECT_FALSE(areCoincident(b, a, maximum));
    }
}

TEST(Point2, TinyEuclideanDisplacementsAvoidSquareUnderflow)
{
    const Point2 diagonal{1.0e-200, 1.0e-200};
    EXPECT_FALSE(areCoincident(Point2{}, diagonal, 1.1e-200));
    EXPECT_TRUE(areCoincident(Point2{}, diagonal, 2.0e-200));
}

TEST(Point2, ArithmeticOverflowIsRejectedForEveryCoordinateAndSign)
{
    for (std::size_t axis = 0; axis < 2; ++axis)
        for (Scalar sign : {-1.0, 1.0})
        {
            std::array<Scalar, 2> values{}, opposite{};
            values[axis] = sign * std::numeric_limits<Scalar>::max();
            opposite[axis] = -values[axis];
            const Point2 point{values[0], values[1]}, other{opposite[0], opposite[1]};
            const Vector2 vector{values[0], values[1]}, negative{opposite[0], opposite[1]};
            EXPECT_THROW((void)(point + vector), std::overflow_error);
            EXPECT_THROW((void)(vector + point), std::overflow_error);
            EXPECT_THROW((void)(point - negative), std::overflow_error);
            EXPECT_THROW((void)(point - other), std::overflow_error);
            EXPECT_TRUE(areCoincident(point, Point2{values[0], values[1]}, 0.0));
        }
}

TEST(Point2, NonFiniteVectorInputsAreInvalidArguments)
{
    for (Scalar value : {std::numeric_limits<Scalar>::quiet_NaN(),
                         std::numeric_limits<Scalar>::infinity(),
                         -std::numeric_limits<Scalar>::infinity()})
        for (std::size_t axis = 0; axis < 2; ++axis)
        {
            std::array<Scalar, 2> values{};
            values[axis] = value;
            const Vector2 vector{values[0], values[1]};
            EXPECT_THROW((void)(Point2{} + vector), std::invalid_argument);
            EXPECT_THROW((void)(vector + Point2{}), std::invalid_argument);
            EXPECT_THROW((void)(Point2{} - vector), std::invalid_argument);
        }
}

}
