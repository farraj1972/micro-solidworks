#include "core/geometry/Arc2.h"
#include "core/geometry/Circle2.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{
using microsw::geometry::Arc2;
using microsw::geometry::Circle2;
using microsw::geometry::Point2;
using microsw::geometry::areCoincident;
using microsw::geometry::defaultGeometricTolerance;
using microsw::math::Scalar;
constexpr auto pi = std::numbers::pi_v<Scalar>;

TEST(Circle2, ValidatesStateAndRetainsValues)
{
    const Circle2 circle{Point2{2, -3}, 4};
    EXPECT_DOUBLE_EQ(circle.center().x(), 2);
    EXPECT_DOUBLE_EQ(circle.center().y(), -3);
    EXPECT_DOUBLE_EQ(circle.radius(), 4);

    for (const auto invalid : {0.0, -1.0, defaultGeometricTolerance,
             std::numeric_limits<Scalar>::infinity(),
             std::numeric_limits<Scalar>::quiet_NaN()})
        EXPECT_THROW((void)(Circle2{Point2{}, invalid}), std::invalid_argument);
}

TEST(Circle2, UsesRadianCounterClockwiseConvention)
{
    const Circle2 circle{Point2{1, 2}, 3};
    EXPECT_TRUE(areCoincident(circle.pointAt(0), Point2{4, 2}, 1e-12));
    EXPECT_TRUE(areCoincident(circle.pointAt(pi / 2), Point2{1, 5}, 1e-12));
    EXPECT_TRUE(areCoincident(circle.pointAt(-pi / 2), Point2{1, -1}, 1e-12));
}

TEST(Circle2, SupportsClosestDistanceAndContains)
{
    const Circle2 circle{Point2{}, 2};
    EXPECT_TRUE(areCoincident(circle.closestPoint(Point2{}), Point2{2, 0}, 0));
    EXPECT_TRUE(areCoincident(circle.closestPoint(Point2{3, 4}), Point2{1.2, 1.6}, 1e-12));
    EXPECT_NEAR(circle.distance(Point2{3, 4}), 3, 1e-12);
    EXPECT_TRUE(circle.contains(Point2{2, 0}, 0));
    EXPECT_TRUE(circle.contains(Point2{2 + 5e-10, 0}));
    EXPECT_FALSE(circle.contains(Point2{2 + 2e-9, 0}));
    EXPECT_THROW((void)circle.contains(Point2{}, -1), std::invalid_argument);
}

TEST(Arc2, ValidatesAndRetainsUnwrappedAngles)
{
    const Arc2 arc{Point2{1, 2}, 3, 7 * pi, -pi / 2};
    EXPECT_DOUBLE_EQ(arc.startAngle(), 7 * pi);
    EXPECT_DOUBLE_EQ(arc.sweepAngle(), -pi / 2);
    EXPECT_THROW((void)(Arc2{Point2{}, 1, 0, 0}), std::invalid_argument);
    EXPECT_THROW((void)(Arc2{Point2{}, 1, 0, 2 * pi}), std::invalid_argument);
    EXPECT_THROW((void)(Arc2{Point2{}, 1, 0, -2 * pi}), std::invalid_argument);
    EXPECT_THROW((void)(Arc2{Point2{}, 1, std::numeric_limits<Scalar>::infinity(), 1}), std::invalid_argument);
}

TEST(Arc2, SupportsCounterClockwiseAndClockwiseEvaluation)
{
    const Arc2 ccw{Point2{}, 2, 0, pi / 2};
    EXPECT_TRUE(areCoincident(ccw.startPoint(), Point2{2, 0}, 1e-12));
    EXPECT_TRUE(areCoincident(ccw.endPoint(), Point2{0, 2}, 1e-12));
    EXPECT_TRUE(areCoincident(ccw.pointAt(0.5), Point2{std::sqrt(2.0), std::sqrt(2.0)}, 1e-12));

    const Arc2 cw{Point2{}, 2, 0, -pi / 2};
    EXPECT_TRUE(areCoincident(cw.endPoint(), Point2{0, -2}, 1e-12));
    EXPECT_TRUE(cw.contains(Point2{std::sqrt(2.0), -std::sqrt(2.0)}, 1e-12));
    EXPECT_FALSE(cw.contains(Point2{0, 2}, 1e-12));
    EXPECT_THROW((void)cw.pointAt(-0.1), std::domain_error);
    EXPECT_THROW((void)cw.pointAt(1.1), std::domain_error);
}

TEST(Arc2, ChoosesRadialPointOrNearestEndpoint)
{
    const Arc2 arc{Point2{}, 2, 0, pi / 2};
    EXPECT_TRUE(areCoincident(arc.closestPoint(Point2{3, 3}),
                              Point2{std::sqrt(2.0), std::sqrt(2.0)}, 1e-12));
    EXPECT_TRUE(areCoincident(arc.closestPoint(Point2{-3, 0}), arc.endPoint(), 1e-12));
    EXPECT_TRUE(areCoincident(arc.closestPoint(Point2{}), arc.startPoint(), 0));
    EXPECT_NEAR(arc.distance(Point2{4, 0}), 2, 1e-12);
    EXPECT_TRUE(arc.contains(arc.pointAt(0.37), 1e-12));
}
}
