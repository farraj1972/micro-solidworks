#include "core/sketch/SketchPlane.h"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

namespace
{
using microsw::geometry::Point2;
using microsw::geometry::Point3;
using microsw::geometry::areCoincident;
using microsw::math::Vector2;
using microsw::math::Vector3;
using microsw::sketch::SketchPlane;

TEST(SketchPlane, DefaultsToGlobalXY)
{
    const SketchPlane plane;
    EXPECT_TRUE(areCoincident(plane.toWorld(Point2{2, 3}), Point3{2, 3, 0}, 0));
    EXPECT_TRUE(microsw::math::almostEqual(plane.normal(), Vector3{0, 0, 1}));
    EXPECT_TRUE(plane.plane().contains(Point3{9, -4, 0}, 0));
}

TEST(SketchPlane, NormalizesArbitraryRightHandedBasis)
{
    const SketchPlane plane{Point3{1, 2, 3}, Vector3{0, 2, 0}, Vector3{0, 0, -4}};
    EXPECT_TRUE(microsw::math::almostEqual(plane.xAxis(), Vector3{0, 1, 0}));
    EXPECT_TRUE(microsw::math::almostEqual(plane.yAxis(), Vector3{0, 0, -1}));
    EXPECT_TRUE(microsw::math::almostEqual(plane.normal(), Vector3{-1, 0, 0}));
    EXPECT_TRUE(areCoincident(plane.toWorld(Point2{5, 7}), Point3{1, 7, -4}, 1e-12));
    EXPECT_TRUE(microsw::math::almostEqual(plane.toWorld(Vector2{5, 7}), Vector3{0, 5, -7}));
}

TEST(SketchPlane, WorldToLocalIsOrthogonalProjection)
{
    const SketchPlane plane{Point3{1, 2, 3}, Vector3{0, 1, 0}, Vector3{0, 0, 1}};
    const auto local = plane.toLocal(Point3{100, 7, -4});
    EXPECT_DOUBLE_EQ(local.x(), 5);
    EXPECT_DOUBLE_EQ(local.y(), -7);
}

TEST(SketchPlane, RejectsInvalidAxes)
{
    EXPECT_THROW((void)(SketchPlane{Point3{}, Vector3{}, Vector3{0, 1, 0}}), std::invalid_argument);
    EXPECT_THROW((void)(SketchPlane{Point3{}, Vector3{1, 0, 0}, Vector3{1, 1, 0}}), std::invalid_argument);
    EXPECT_THROW((void)(SketchPlane{Point3{}, Vector3{1, 0, 0},
        Vector3{0, std::numeric_limits<double>::infinity(), 0}}), std::invalid_argument);
}
}
