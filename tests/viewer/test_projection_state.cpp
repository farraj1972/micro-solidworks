#include "viewer/ProjectionState.h"
#include "viewer/PanZoomNavigation.h"
#include "viewer/OrbitNavigation.h"
#include "viewer/ViewProjection.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using microsw::ProjectionMode;
using microsw::viewer::ProjectionState;
using microsw::viewer::PanZoomNavigation;
using microsw::viewer::OrbitNavigation;
using microsw::viewer::OrbitCamera;
using microsw::math::almostEqual;
constexpr microsw::WorkspaceLayout layout{260, 20, 740, 650, 1000, 700};

microsw::WorkspaceInput wheel(double delta)
{
    return {500, 300, false, false, false, true, true, true, false, delta};
}
void drag(PanZoomNavigation& navigation, OrbitCamera& camera, ProjectionState& projection)
{
    auto input = wheel(0);
    input.shiftDown = input.middleDown = input.middlePressed = true;
    navigation.handle(layout, input, camera, projection);
    input.middlePressed = false;
    input.x += 20;
    input.y -= 10;
    navigation.handle(layout, input, camera, projection);
}
void expectPose(const OrbitCamera& actual, const OrbitCamera& expected)
{
    EXPECT_TRUE(almostEqual(actual.target(), expected.target()));
    EXPECT_TRUE(almostEqual(actual.yaw(), expected.yaw()));
    EXPECT_TRUE(almostEqual(actual.pitch(), expected.pitch()));
    EXPECT_TRUE(almostEqual(actual.distance(), expected.distance()));
}

TEST(ProjectionStateTest, DefaultsAndIdempotentSwitches)
{
    ProjectionState state;
    EXPECT_EQ(state.mode(), ProjectionMode::Perspective);
    EXPECT_DOUBLE_EQ(state.visibleHeight(), 10);
    for (auto mode : {ProjectionMode::Orthographic, ProjectionMode::Orthographic,
                     ProjectionMode::Perspective, ProjectionMode::Perspective})
    {
        state.setMode(mode);
        EXPECT_EQ(state.mode(), mode);
        EXPECT_DOUBLE_EQ(state.visibleHeight(), 10);
    }
}

TEST(ProjectionStateTest, HeightRejectsInvalidValuesWithoutChangingState)
{
    ProjectionState state;
    state.setVisibleHeight(5);
    for (double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                            std::numeric_limits<double>::quiet_NaN()})
    {
        EXPECT_THROW(state.setVisibleHeight(invalid), std::invalid_argument);
        EXPECT_DOUBLE_EQ(state.visibleHeight(), 5);
    }
}

TEST(ProjectionStateTest, MatrixSelectionReusesBothMathFunctions)
{
    ProjectionState state;
    state.setVisibleHeight(7);
    EXPECT_TRUE(almostEqual(state.matrix(1.0, 1.5, 0.1, 1100),
        microsw::viewer::perspective(1.0, 1.5, 0.1, 1100)));
    state.setMode(ProjectionMode::Orthographic);
    EXPECT_TRUE(almostEqual(state.matrix(1.0, 1.5, 0.1, 1100),
        microsw::viewer::orthographic(7, 1.5, 0.1, 1100)));
}

TEST(ProjectionStateTest, OnlyPerspectiveApparentSizeDependsOnDepth)
{
    ProjectionState state;
    auto projectedX = [](const microsw::math::Matrix4& m, double z) {
        return (m(0, 0) + m(0, 2) * z + m(0, 3))
            / (m(3, 0) + m(3, 2) * z + m(3, 3));
    };
    const auto perspective = state.matrix(1.0, 1.0, 0.1, 1100);
    EXPECT_TRUE(almostEqual(projectedX(perspective, -5), 2 * projectedX(perspective, -10)));
    state.setMode(ProjectionMode::Orthographic);
    const auto orthographic = state.matrix(1.0, 1.0, 0.1, 1100);
    EXPECT_TRUE(almostEqual(projectedX(orthographic, -5), projectedX(orthographic, -10)));
}

TEST(ProjectionStateTest, SwitchingPreservesCameraAndIndependentZoomStates)
{
    OrbitCamera camera{{2, -3, 4}, 20, 0.8, 0.3};
    const auto before = camera;
    ProjectionState state;
    state.setVisibleHeight(5);
    for (auto mode : {ProjectionMode::Orthographic, ProjectionMode::Perspective,
                     ProjectionMode::Orthographic})
    {
        state.setMode(mode);
        expectPose(camera, before);
        EXPECT_DOUBLE_EQ(state.visibleHeight(), 5);
    }
}

TEST(ProjectionNavigationTest, OrthographicWheelChangesOnlyHeightInBothDirections)
{
    OrbitCamera camera{{2, 3, 4}, 20, 0.5, 0.2};
    const auto before = camera;
    ProjectionState state;
    state.setMode(ProjectionMode::Orthographic);
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(1), camera, state);
    EXPECT_LT(state.visibleHeight(), 10);
    EXPECT_TRUE(almostEqual(state.visibleHeight(), 10 * std::exp(-navigation.zoomSensitivity())));
    expectPose(camera, before);
    navigation.handle(layout, wheel(-2), camera, state);
    EXPECT_GT(state.visibleHeight(), 10);
    expectPose(camera, before);
}

TEST(ProjectionNavigationTest, OrthographicZoomClampsBothBoundsWithoutChangingCamera)
{
    OrbitCamera camera;
    const auto before = camera;
    ProjectionState state;
    state.setMode(ProjectionMode::Orthographic);
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(std::numeric_limits<double>::max()), camera, state);
    EXPECT_DOUBLE_EQ(state.visibleHeight(), navigation.minimumVisibleHeight());
    navigation.handle(layout, wheel(-1), camera, state);
    EXPECT_GT(state.visibleHeight(), navigation.minimumVisibleHeight());
    navigation.handle(layout, wheel(-std::numeric_limits<double>::max()), camera, state);
    EXPECT_DOUBLE_EQ(state.visibleHeight(), navigation.maximumVisibleHeight());
    expectPose(camera, before);
}

TEST(ProjectionNavigationTest, OrthographicZoomHonorsUIFocusAndWorkspaceGates)
{
    for (int reason = 0; reason < 5; ++reason)
    {
        ProjectionState state;
        state.setMode(ProjectionMode::Orthographic);
        OrbitCamera camera;
        PanZoomNavigation navigation;
        auto input = wheel(1);
        if (reason == 0) input.blocked = true;
        if (reason == 1) input.focused = false;
        if (reason == 2) input.pointerValid = false;
        if (reason == 3) input.workspaceHovered = false;
        if (reason == 4) input.x = 100;
        navigation.handle(layout, input, camera, state);
        EXPECT_DOUBLE_EQ(state.visibleHeight(), 10);
        EXPECT_DOUBLE_EQ(camera.distance(), 10);
    }
}

TEST(ProjectionNavigationTest, OrthographicZoomIgnoresNonFiniteWheel)
{
    ProjectionState state;
    state.setMode(ProjectionMode::Orthographic);
    OrbitCamera camera;
    PanZoomNavigation navigation;
    for (double delta : {std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()})
        navigation.handle(layout, wheel(delta), camera, state);
    EXPECT_DOUBLE_EQ(state.visibleHeight(), 10);
}

TEST(ProjectionNavigationTest, OrthographicPanUsesHeightNotDistance)
{
    OrbitCamera first{{}, 10, 0.7, 0.2}, second{{}, 50, 0.7, 0.2};
    ProjectionState state;
    state.setMode(ProjectionMode::Orthographic);
    state.setVisibleHeight(6);
    const auto before = first;
    PanZoomNavigation navigation;
    drag(navigation, first, state);
    drag(navigation, second, state);
    EXPECT_TRUE(almostEqual(first.target(), second.target()));
    const auto expected = before.right() * (-20 * 6 * navigation.panSensitivity())
        + before.up() * (-10 * 6 * navigation.panSensitivity());
    EXPECT_TRUE(almostEqual(first.target(), expected));
    EXPECT_DOUBLE_EQ(first.distance(), 10);
    EXPECT_DOUBLE_EQ(second.distance(), 50);
    EXPECT_DOUBLE_EQ(first.yaw(), before.yaw());
    EXPECT_DOUBLE_EQ(first.pitch(), before.pitch());
    EXPECT_DOUBLE_EQ(state.visibleHeight(), 6);
}

TEST(ProjectionNavigationTest, OrthographicPanIsProportionalToHeight)
{
    OrbitCamera first, second;
    ProjectionState state;
    state.setMode(ProjectionMode::Orthographic);
    PanZoomNavigation navigation;
    state.setVisibleHeight(5);
    drag(navigation, first, state);
    state.setVisibleHeight(10);
    drag(navigation, second, state);
    EXPECT_TRUE(almostEqual(second.target(), first.target() * 2));
}

TEST(ProjectionNavigationTest, PerspectivePanAndZoomIgnoreStoredOrthographicHeight)
{
    OrbitCamera first{{}, 10, 0.5, 0.3}, second{{}, 20, 0.5, 0.3};
    ProjectionState state;
    state.setVisibleHeight(7);
    PanZoomNavigation navigation;
    drag(navigation, first, state);
    drag(navigation, second, state);
    EXPECT_TRUE(almostEqual(second.target(), first.target() * 2));
    navigation.handle(layout, wheel(1), first, state);
    EXPECT_TRUE(almostEqual(first.distance(), 10 * std::exp(-navigation.zoomSensitivity())));
    EXPECT_DOUBLE_EQ(state.visibleHeight(), 7);
}

TEST(ProjectionNavigationTest, EachModeRestoresItsLastZoomAfterRepeatedSwitches)
{
    OrbitCamera camera;
    ProjectionState state;
    PanZoomNavigation navigation;
    navigation.handle(layout, wheel(-2), camera, state);
    const auto perspectiveDistance = camera.distance();
    state.setMode(ProjectionMode::Orthographic);
    navigation.handle(layout, wheel(3), camera, state);
    const auto height = state.visibleHeight();
    const auto before = camera;
    for (int i = 0; i < 3; ++i)
    {
        state.setMode(ProjectionMode::Perspective);
        EXPECT_DOUBLE_EQ(camera.distance(), perspectiveDistance);
        state.setMode(ProjectionMode::Orthographic);
        EXPECT_DOUBLE_EQ(state.visibleHeight(), height);
        expectPose(camera, before);
    }
}

TEST(ProjectionNavigationTest, OrbitWorksInBothModesWithoutChangingZoomStates)
{
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        OrbitCamera camera;
        const auto before = camera;
        ProjectionState state;
        state.setMode(mode);
        OrbitNavigation orbit;
        PanZoomNavigation panZoom;
        auto input = wheel(0);
        input.middlePressed = input.middleDown = true;
        orbit.handle(layout, input, camera);
        panZoom.handle(layout, input, camera, state);
        input.middlePressed = false;
        input.x += 20;
        input.y -= 10;
        orbit.handle(layout, input, camera);
        panZoom.handle(layout, input, camera, state);
        EXPECT_GT(camera.yaw(), before.yaw());
        EXPECT_GT(camera.pitch(), before.pitch());
        EXPECT_TRUE(almostEqual(camera.target(), before.target()));
        EXPECT_DOUBLE_EQ(camera.distance(), before.distance());
        EXPECT_DOUBLE_EQ(state.visibleHeight(), 10);
    }
}
}
