#include "viewer/ReferenceAxes.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>
#include <cmath>

namespace
{

using microsw::viewer::ReferenceAxes;
using microsw::math::Vector3;
using microsw::math::almostEqual;
using microsw::math::dot;
using microsw::math::cross;

TEST(ReferenceAxesTest, AllSegmentsStartAtOrigin)
{
    const ReferenceAxes axes;
    for (const auto& segment : {axes.xAxis(), axes.yAxis(), axes.zAxis()})
        EXPECT_TRUE(almostEqual(segment[0], Vector3{}));
}

TEST(ReferenceAxesTest, EndpointsFollowPositiveCoordinateAxes)
{
    const ReferenceAxes axes;
    const auto length = axes.length();
    EXPECT_TRUE(almostEqual(axes.xAxis()[1], Vector3{length, 0.0, 0.0}));
    EXPECT_TRUE(almostEqual(axes.yAxis()[1], Vector3{0.0, length, 0.0}));
    EXPECT_TRUE(almostEqual(axes.zAxis()[1], Vector3{0.0, 0.0, length}));
}

TEST(ReferenceAxesTest, LengthIsPositiveFiniteAndShared)
{
    const ReferenceAxes axes;
    EXPECT_TRUE(std::isfinite(axes.length()));
    EXPECT_GT(axes.length(), 0.0);
    for (const auto& segment : {axes.xAxis(), axes.yAxis(), axes.zAxis()})
    {
        EXPECT_TRUE(almostEqual((segment[1] - segment[0]).length(), axes.length()));
        for (const auto& vertex : segment)
        {
            EXPECT_TRUE(std::isfinite(vertex.x()));
            EXPECT_TRUE(std::isfinite(vertex.y()));
            EXPECT_TRUE(std::isfinite(vertex.z()));
        }
    }
}

TEST(ReferenceAxesTest, DirectionsAreOrthogonal)
{
    const ReferenceAxes axes;
    const auto x = axes.xAxis()[1] - axes.xAxis()[0];
    const auto y = axes.yAxis()[1] - axes.yAxis()[0];
    const auto z = axes.zAxis()[1] - axes.zAxis()[0];
    EXPECT_TRUE(almostEqual(dot(x, y), 0.0));
    EXPECT_TRUE(almostEqual(dot(y, z), 0.0));
    EXPECT_TRUE(almostEqual(dot(z, x), 0.0));
}

TEST(ReferenceAxesTest, XYCrossProductPointsAlongPositiveZ)
{
    const ReferenceAxes axes;
    const auto x = axes.xAxis()[1] - axes.xAxis()[0];
    const auto y = axes.yAxis()[1] - axes.yAxis()[0];
    const auto z = axes.zAxis()[1] - axes.zAxis()[0];
    EXPECT_TRUE(almostEqual(cross(x, y).normalized(), z.normalized()));
    EXPECT_GT(dot(cross(x, y), z), 0.0);
}

}
