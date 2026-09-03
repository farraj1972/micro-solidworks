#include "viewer/OrbitNavigation.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <numbers>

namespace
{
using microsw::viewer::OrbitCamera;
using microsw::viewer::OrbitNavigation;
using microsw::viewer::contains;
using microsw::math::almostEqual;
using microsw::WorkspaceLayout;
using microsw::WorkspaceInput;
constexpr double pi = std::numbers::pi;
constexpr WorkspaceLayout layout{260, 20, 740, 650, 1000, 700};

WorkspaceInput press(double x = 500, double y = 300)
{
    return {x, y, true, true, false, true, true, true, false};
}

TEST(OrbitNavigationTest, BeginActivatesWithoutMovingCamera)
{
    OrbitCamera camera;
    const auto position = camera.position();
    OrbitNavigation navigation;
    EXPECT_FALSE(navigation.active());
    navigation.handle(layout, press(), camera);
    EXPECT_TRUE(navigation.active());
    EXPECT_TRUE(almostEqual(position, camera.position()));
    navigation.update(500, 300, camera);
    EXPECT_TRUE(almostEqual(position, camera.position()));
}

TEST(OrbitNavigationTest, RightAndLeftHaveExplicitYawSigns)
{
    OrbitCamera camera{{}, 10, 0, 0};
    OrbitNavigation navigation;
    navigation.begin(100, 100);
    navigation.update(120, 100, camera);
    EXPECT_TRUE(almostEqual(camera.yaw(), 20 * navigation.sensitivity()));
    navigation.update(80, 100, camera);
    EXPECT_TRUE(almostEqual(camera.yaw(), -20 * navigation.sensitivity()));
    EXPECT_TRUE(almostEqual(camera.pitch(), 0.0));
}

TEST(OrbitNavigationTest, UpAndDownHaveExplicitPitchSigns)
{
    OrbitCamera camera{{}, 10, 0, 0};
    OrbitNavigation navigation;
    navigation.begin(100, 100);
    navigation.update(100, 80, camera);
    EXPECT_TRUE(almostEqual(camera.pitch(), 20 * navigation.sensitivity()));
    navigation.update(100, 120, camera);
    EXPECT_TRUE(almostEqual(camera.pitch(), -20 * navigation.sensitivity()));
    EXPECT_TRUE(almostEqual(camera.yaw(), 0.0));
}

TEST(OrbitNavigationTest, TargetDistanceAndRadiusArePreserved)
{
    OrbitCamera camera{{2, -3, 4}, 7, 0.2, 0.1};
    const auto target = camera.target();
    const auto distance = camera.distance();
    OrbitNavigation navigation;
    navigation.begin(0, 0);
    for (int i = 1; i <= 100; ++i)
    {
        navigation.update(i * 5.0, i * -2.0, camera);
        EXPECT_TRUE(almostEqual(camera.target(), target));
        EXPECT_TRUE(almostEqual(camera.distance(), distance));
        EXPECT_TRUE(almostEqual((camera.position() - target).length(), distance));
    }
}

TEST(OrbitNavigationTest, QuarterTurnReachesPositiveY)
{
    OrbitCamera camera{{}, 10, 0, 0};
    OrbitNavigation navigation;
    navigation.begin(0, 0);
    navigation.update((pi / 2) / navigation.sensitivity(), 0, camera);
    EXPECT_TRUE(almostEqual(camera.position(), microsw::math::Vector3{0, 10, 0}));
}

TEST(OrbitNavigationTest, YawRemainsUnwrapped)
{
    OrbitCamera camera{{}, 10, 0, 0};
    OrbitNavigation navigation;
    navigation.begin(0, 0);
    navigation.update(10 * pi / navigation.sensitivity(), 0, camera);
    EXPECT_TRUE(almostEqual(camera.yaw(), 10 * pi));
}

TEST(OrbitNavigationTest, BothPolesUseCameraClampAndKeepFiniteBasis)
{
    for (double y : {-1.0e6, 1.0e6})
    {
        OrbitCamera camera{{}, 10, 0, 0};
        OrbitCamera expected = camera;
        OrbitNavigation navigation;
        navigation.begin(0, 0);
        navigation.update(0, y, camera);
        expected.setPitch(-y * navigation.sensitivity());
        EXPECT_TRUE(almostEqual(camera.pitch(), expected.pitch()));
        EXPECT_LT(camera.pitch(), pi / 2);
        EXPECT_GT(camera.pitch(), -pi / 2);
        for (const auto vector : {camera.forward(), camera.right(), camera.up()})
        {
            EXPECT_TRUE(std::isfinite(vector.x()));
            EXPECT_TRUE(std::isfinite(vector.y()));
            EXPECT_TRUE(std::isfinite(vector.z()));
            EXPECT_TRUE(almostEqual(vector.length(), 1.0));
        }
        // No accumulated overshoot: a small reversed drag immediately leaves clamp.
        navigation.update(0, y + (y < 0 ? 1 : -1), camera);
        EXPECT_LT(std::abs(camera.pitch()), std::abs(expected.pitch()));
    }
}

TEST(OrbitNavigationTest, EndAndInactiveUpdatesDoNotMoveCamera)
{
    OrbitCamera camera;
    const auto position = camera.position();
    OrbitNavigation navigation;
    navigation.update(100, 200, camera);
    navigation.begin(0, 0);
    navigation.end();
    EXPECT_FALSE(navigation.active());
    navigation.update(100, 200, camera);
    EXPECT_TRUE(almostEqual(position, camera.position()));
}

TEST(OrbitNavigationTest, RebeginResetsAnchorWithoutJump)
{
    OrbitCamera camera{{}, 10, 0, 0};
    OrbitNavigation navigation;
    navigation.begin(0, 0);
    navigation.update(10, 0, camera);
    navigation.begin(1000, 1000);
    navigation.update(1010, 1000, camera);
    EXPECT_TRUE(almostEqual(camera.yaw(), 20 * navigation.sensitivity()));
}

TEST(OrbitNavigationTest, InvalidCoordinatesAndDeltaOverflowCancelAtomically)
{
    for (double invalid : {std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::infinity()})
    {
        OrbitNavigation navigation;
        OrbitCamera camera;
        const auto position = camera.position();
        navigation.begin(invalid, 0);
        EXPECT_FALSE(navigation.active());
        navigation.begin(0, 0);
        navigation.update(10, invalid, camera);
        EXPECT_FALSE(navigation.active());
        EXPECT_TRUE(almostEqual(position, camera.position()));
    }
    OrbitCamera camera;
    OrbitNavigation navigation;
    const auto position = camera.position();
    navigation.begin(-std::numeric_limits<double>::max(), 0);
    navigation.update(std::numeric_limits<double>::max(), 0, camera);
    EXPECT_FALSE(navigation.active());
    EXPECT_TRUE(almostEqual(position, camera.position()));
}

TEST(OrbitNavigationTest, PressOutsideCannotBecomeDragOnEnteringWorkspace)
{
    for (auto input : {press(100, 300), press(500, 10), press(500, 690), press(1100, 300)})
    {
        OrbitCamera camera;
        const auto position = camera.position();
        OrbitNavigation navigation;
        navigation.handle(layout, input, camera);
        input = press();
        input.middlePressed = false;
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
        EXPECT_TRUE(almostEqual(position, camera.position()));
    }
}

TEST(OrbitNavigationTest, WorkspaceHoverPermissionIsRequiredToStart)
{
    OrbitCamera camera;
    OrbitNavigation navigation;
    auto input = press();
    input.workspaceHovered = false;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
}

TEST(OrbitNavigationTest, ActiveGestureContinuesOutsideAndReleaseStopsIt)
{
    OrbitCamera camera;
    OrbitNavigation navigation;
    navigation.handle(layout, press(), camera);
    const double yaw = camera.yaw();
    auto input = press(1100, 300);
    input.middlePressed = false;
    input.workspaceHovered = false;
    navigation.handle(layout, input, camera);
    EXPECT_TRUE(navigation.active());
    EXPECT_GT(camera.yaw(), yaw);
    input.middleDown = false;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
    const auto position = camera.position();
    input.x = 400;
    navigation.handle(layout, input, camera);
    EXPECT_TRUE(almostEqual(position, camera.position()));
}

TEST(OrbitNavigationTest, FocusUIShiftAndInvalidPointerCancelAndRequireNewPress)
{
    for (int reason = 0; reason < 4; ++reason)
    {
        OrbitCamera camera;
        OrbitNavigation navigation;
        navigation.handle(layout, press(), camera);
        auto input = press(600, 200);
        input.middlePressed = false;
        if (reason == 0) input.focused = false;
        if (reason == 1) input.blocked = true;
        if (reason == 2) input.shiftDown = true;
        if (reason == 3) input.pointerValid = false;
        const auto position = camera.position();
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
        EXPECT_TRUE(almostEqual(position, camera.position()));
        input = press();
        input.middlePressed = false;
        navigation.handle(layout, input, camera);
        EXPECT_FALSE(navigation.active());
    }
}

TEST(OrbitNavigationTest, BlockedOrShiftPressCannotStart)
{
    OrbitCamera camera;
    OrbitNavigation navigation;
    auto input = press();
    input.blocked = true;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
    input.blocked = false;
    input.shiftDown = true;
    navigation.handle(layout, input, camera);
    EXPECT_FALSE(navigation.active());
}

TEST(WorkspaceHitTest, InteriorAndHalfOpenBoundaries)
{
    EXPECT_TRUE(contains(layout, 500, 300));
    EXPECT_TRUE(contains(layout, 260, 20));
    EXPECT_TRUE(contains(layout, 999, 669));
    EXPECT_FALSE(contains(layout, 1000, 300));
    EXPECT_FALSE(contains(layout, 500, 670));
}

TEST(WorkspaceHitTest, OutsideAllSides)
{
    EXPECT_FALSE(contains(layout, 259, 300));
    EXPECT_FALSE(contains(layout, 1001, 300));
    EXPECT_FALSE(contains(layout, 500, 19));
    EXPECT_FALSE(contains(layout, 500, 671));
}

TEST(WorkspaceHitTest, EmptyInvalidAndNonFiniteLayouts)
{
    EXPECT_FALSE(contains({}, 0, 0));
    for (double value : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                          std::numeric_limits<double>::quiet_NaN()})
    {
        auto invalid = layout;
        invalid.width = value;
        EXPECT_FALSE(contains(invalid, 500, 300));
        invalid = layout;
        invalid.height = value;
        EXPECT_FALSE(contains(invalid, 500, 300));
        invalid = layout;
        invalid.displayWidth = value;
        EXPECT_FALSE(contains(invalid, 500, 300));
    }
    EXPECT_FALSE(contains(layout, std::numeric_limits<double>::quiet_NaN(), 300));
}

TEST(WorkspaceHitTest, LogicalCoordinatesClipToDisplay)
{
    const WorkspaceLayout clipped{-10, -20, 200, 200, 100, 100};
    EXPECT_TRUE(contains(clipped, 0, 0));
    EXPECT_FALSE(contains(clipped, -1, 0));
    EXPECT_FALSE(contains(clipped, 100, 50));
}
}
