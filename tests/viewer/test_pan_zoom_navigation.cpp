#include "viewer/PanZoomNavigation.h"
#include "viewer/OrbitNavigation.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>
#include <cmath>
#include <limits>

namespace
{
using microsw::viewer::PanZoomNavigation;
using microsw::viewer::OrbitNavigation;
using microsw::viewer::OrbitCamera;
using microsw::math::almostEqual;
using microsw::math::dot;
using microsw::WorkspaceInput;
constexpr microsw::WorkspaceLayout layout{260, 20, 740, 650, 1000, 700};

WorkspaceInput panPress()
{
    return {500, 300, true, true, true, true, true, true, false, 0};
}
WorkspaceInput wheel(double delta)
{
    auto input = panPress();
    input.middlePressed = input.middleDown = input.shiftDown = false;
    input.wheelDelta = delta;
    return input;
}
void pan(PanZoomNavigation& navigation, OrbitCamera& camera, double dx, double dy)
{
    auto input = panPress();
    navigation.handle(layout, input, camera);
    input.middlePressed = false;
    input.x += dx;
    input.y += dy;
    navigation.handle(layout, input, camera);
}

TEST(PanNavigationTest, ShiftPressActivatesWithoutFirstFrameJump)
{
    OrbitCamera camera;
    const auto position = camera.position();
    PanZoomNavigation navigation;
    EXPECT_FALSE(navigation.active());
    navigation.handle(layout, panPress(), camera);
    EXPECT_TRUE(navigation.active());
    EXPECT_TRUE(almostEqual(camera.position(), position));
}

TEST(PanNavigationTest, HorizontalSignsFollowCursor)
{
    for (double dx : {-20.0, 20.0})
    {
        OrbitCamera camera;
        const auto right = camera.right();
        PanZoomNavigation navigation;
        pan(navigation, camera, dx, 0);
        EXPECT_TRUE(almostEqual(camera.target(), right * (-dx * 10 * navigation.panSensitivity())));
    }
}

TEST(PanNavigationTest, VerticalSignsFollowCursor)
{
    for (double dy : {-20.0, 20.0})
    {
        OrbitCamera camera;
        const auto up = camera.up();
        PanZoomNavigation navigation;
        pan(navigation, camera, 0, dy);
        EXPECT_TRUE(almostEqual(camera.target(), up * (dy * 10 * navigation.panSensitivity())));
    }
}

TEST(PanNavigationTest, PreservesOrientationDistanceAndTranslatesPositionWithTarget)
{
    OrbitCamera camera{{2, 3, -4}, 15, 1.2, -0.4};
    const OrbitCamera before = camera;
    PanZoomNavigation navigation;
    pan(navigation, camera, 37, -23);
    const auto delta = camera.target() - before.target();
    EXPECT_TRUE(almostEqual(camera.yaw(), before.yaw()));
    EXPECT_TRUE(almostEqual(camera.pitch(), before.pitch()));
    EXPECT_TRUE(almostEqual(camera.distance(), before.distance()));
    EXPECT_TRUE(almostEqual(camera.right(), before.right()));
    EXPECT_TRUE(almostEqual(camera.up(), before.up()));
    EXPECT_TRUE(almostEqual(camera.forward(), before.forward()));
    EXPECT_TRUE(almostEqual(camera.position() - before.position(), delta));
    EXPECT_TRUE(almostEqual(dot(delta, before.forward()), 0.0));
}

TEST(PanNavigationTest, ScaleIsProportionalToDistance)
{
    OrbitCamera nearCamera{{}, 10, 0.7, 0.2};
    OrbitCamera farCamera{{}, 20, 0.7, 0.2};
    PanZoomNavigation first, second;
    pan(first, nearCamera, 50, 30);
    pan(second, farCamera, 50, 30);
    EXPECT_TRUE(almostEqual(farCamera.target(), nearCamera.target() * 2));
}

TEST(PanNavigationTest, CapturedPanContinuesOutsideAndReleaseStops)
{
    OrbitCamera camera;
    PanZoomNavigation navigation;
    auto input = panPress();
    navigation.handle(layout, input, camera);
    input.middlePressed = false;
    input.workspaceHovered = false;
    input.x = 100;
    navigation.handle(layout, input, camera);
    EXPECT_TRUE(navigation.active());
    EXPECT_FALSE(almostEqual(camera.target(), microsw::math::Vector3{}));
    const auto target = camera.target();
    input.middleDown = false;
    input.x = 0;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
    EXPECT_TRUE(almostEqual(camera.target(), target));
}

TEST(PanNavigationTest, OutsidePressCannotStartAfterEnteringWhileHeld)
{
    for (const auto point : {microsw::math::Vector3{100, 300, 0},
                            {500, 10, 0}, {500, 690, 0}, {1100, 300, 0}})
    {
        OrbitCamera camera;
        PanZoomNavigation navigation;
        auto input = panPress();
        input.x = point.x();
        input.y = point.y();
        navigation.handle(layout, input, camera);
        input = panPress();
        input.middlePressed = false;
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
        EXPECT_TRUE(almostEqual(camera.target(), microsw::math::Vector3{}));
    }
}

TEST(PanNavigationTest, FocusBlockPointerAndShiftCancelWithoutResuming)
{
    for (int reason = 0; reason < 4; ++reason)
    {
        OrbitCamera camera;
        PanZoomNavigation navigation;
        auto input = panPress();
        navigation.handle(layout, input, camera);
        input.middlePressed = false;
        input.x += 50;
        if (reason == 0) input.focused = false;
        if (reason == 1) input.blocked = true;
        if (reason == 2) input.pointerValid = false;
        if (reason == 3) input.shiftDown = false;
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
        EXPECT_TRUE(almostEqual(camera.target(), microsw::math::Vector3{}));
        input = panPress();
        input.middlePressed = false;
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
    }
}

TEST(PanNavigationTest, PlainMMBAndUIHoverCannotStartPan)
{
    OrbitCamera camera;
    PanZoomNavigation navigation;
    auto input = panPress();
    input.shiftDown = false;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
    input = panPress();
    input.workspaceHovered = false;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
}

TEST(PanNavigationTest, InvalidMotionCancelsWithoutChangingTarget)
{
    for (double invalid : {std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::infinity()})
    {
        OrbitCamera camera;
        PanZoomNavigation navigation;
        auto input = panPress();
        navigation.handle(layout, input, camera);
        input.middlePressed = false;
        input.x = invalid;
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
        EXPECT_TRUE(almostEqual(camera.target(), microsw::math::Vector3{}));
    }
    OrbitCamera camera{{}, std::numeric_limits<double>::max(), 0, 0};
    PanZoomNavigation navigation;
    pan(navigation, camera, 10000, 0);
    EXPECT_FALSE(navigation.active());
    EXPECT_TRUE(almostEqual(camera.target(), microsw::math::Vector3{}));
}

TEST(ZoomNavigationTest, UpDecreasesAndDownIncreasesDistance)
{
    OrbitCamera camera;
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(1), camera);
    EXPECT_LT(camera.distance(), 10);
    EXPECT_TRUE(almostEqual(camera.distance(), 10 * std::exp(-navigation.zoomSensitivity())));
    navigation.handle(layout, wheel(-1), camera);
    EXPECT_TRUE(almostEqual(camera.distance(), 10.0));
    navigation.handle(layout, wheel(-1), camera);
    EXPECT_GT(camera.distance(), 10);
}

TEST(ZoomNavigationTest, PreservesTargetYawAndPitch)
{
    OrbitCamera camera{{2, -3, 4}, 10, 1.3, 0.2};
    const auto before = camera;
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(2), camera);
    EXPECT_TRUE(almostEqual(camera.target(), before.target()));
    EXPECT_TRUE(almostEqual(camera.yaw(), before.yaw()));
    EXPECT_TRUE(almostEqual(camera.pitch(), before.pitch()));
}

TEST(ZoomNavigationTest, MultiplicativeAtDifferentDistancesAndFractionalSteps)
{
    OrbitCamera first{{}, 5, 0, 0}, second{{}, 50, 0, 0};
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(0.5), first);
    navigation.handle(layout, wheel(0.5), second);
    EXPECT_TRUE(almostEqual(first.distance() / 5, second.distance() / 50));
    EXPECT_TRUE(almostEqual(first.distance(), 5 * std::exp(-0.5 * navigation.zoomSensitivity())));
}

TEST(ZoomNavigationTest, BothLimitsClampExtremeFiniteInputAndAllowReversal)
{
    OrbitCamera camera;
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(std::numeric_limits<double>::max()), camera);
    EXPECT_DOUBLE_EQ(camera.distance(), navigation.minimumDistance());
    navigation.handle(layout, wheel(-1), camera);
    EXPECT_GT(camera.distance(), navigation.minimumDistance());
    navigation.handle(layout, wheel(-std::numeric_limits<double>::max()), camera);
    EXPECT_DOUBLE_EQ(camera.distance(), navigation.maximumDistance());
    navigation.handle(layout, wheel(1), camera);
    EXPECT_LT(camera.distance(), navigation.maximumDistance());
}

TEST(ZoomNavigationTest, IgnoredOutsideWorkspaceAndWithoutHoverPermission)
{
    for (int region = 0; region < 5; ++region)
    {
        OrbitCamera camera;
        PanZoomNavigation navigation;
        auto input = wheel(1);
        if (region == 0) input.x = 100;
        if (region == 1) input.y = 10;
        if (region == 2) input.y = 690;
        if (region == 3) input.x = 1100;
        if (region == 4) input.workspaceHovered = false;
        navigation.handle(layout, input, camera);
        EXPECT_DOUBLE_EQ(camera.distance(), 10.0);
    }
}

TEST(ZoomNavigationTest, IgnoredWhenBlockedUnfocusedOrPointerInvalid)
{
    for (int reason = 0; reason < 3; ++reason)
    {
        OrbitCamera camera;
        PanZoomNavigation navigation;
        auto input = wheel(1);
        if (reason == 0) input.blocked = true;
        if (reason == 1) input.focused = false;
        if (reason == 2) input.pointerValid = false;
        navigation.handle(layout, input, camera);
        EXPECT_DOUBLE_EQ(camera.distance(), 10.0);
    }
}

TEST(ZoomNavigationTest, ZeroAndNonFiniteWheelAreIgnored)
{
    OrbitCamera camera;
    PanZoomNavigation navigation;
    for (double delta : {0.0, std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()})
        navigation.handle(layout, wheel(delta), camera);
    EXPECT_DOUBLE_EQ(camera.distance(), 10.0);
}

TEST(NavigationMappingTest, OrbitPanSwitchRequiresReleaseAndNewPress)
{
    for (bool startPan : {false, true})
    {
        OrbitCamera camera;
        OrbitNavigation orbit;
        PanZoomNavigation panZoom;
        auto input = panPress();
        input.shiftDown = startPan;
        auto handle = [&] {
            orbit.handle(layout, input, camera);
            panZoom.handle(layout, input, camera);
        };
        handle();
        EXPECT_EQ(orbit.active(), !startPan);
        EXPECT_EQ(panZoom.active(), startPan);
        input.middlePressed = false;
        input.x += 20;
        handle();
        if (startPan)
            EXPECT_TRUE(almostEqual(camera.yaw(), OrbitCamera{}.yaw()));
        else
            EXPECT_TRUE(almostEqual(camera.target(), microsw::math::Vector3{}));
        const auto before = camera;
        input.shiftDown = !startPan;
        input.x += 20;
        handle();
        EXPECT_FALSE(orbit.active());
        EXPECT_FALSE(panZoom.active());
        EXPECT_TRUE(almostEqual(camera.position(), before.position()));
        input.shiftDown = startPan;
        handle();
        EXPECT_FALSE(orbit.active());
        EXPECT_FALSE(panZoom.active());
        input.middleDown = false;
        handle();
        input.middleDown = input.middlePressed = true;
        handle();
        EXPECT_EQ(orbit.active(), !startPan);
        EXPECT_EQ(panZoom.active(), startPan);
    }
}
}
