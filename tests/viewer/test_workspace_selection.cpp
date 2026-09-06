#include "viewer/WorkspaceViewport.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "presentation/GeometryPresentation.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <numbers>

namespace
{
using microsw::WorkspaceLayout;
using microsw::WorkspaceInput;
using microsw::ProjectionMode;
using microsw::viewer::WorkspaceViewport;
using microsw::viewer::OrbitCamera;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::Line3;
using microsw::presentation::GeometryPresentation;

class WorkspaceSelectionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<microsw::ApplicationWindow>(128, 128, "Selection tests");
        glfwHideWindow(static_cast<GLFWwindow*>(window->nativeHandle()));
        context = std::make_unique<microsw::OpenGLContext>(*window);
    }
    void TearDown() override
    {
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
        context.reset();
        window.reset();
    }
    void frame(WorkspaceViewport& workspace, const WorkspaceInput& snapshot)
    {
        workspace.updateNavigation(layout, snapshot);
        workspace.updateHover(layout, snapshot, 128, 128);
        workspace.updateSelection(layout, snapshot, 128, 128);
    }
    void click(WorkspaceViewport& workspace, WorkspaceInput snapshot)
    {
        snapshot.leftPressed = true;
        frame(workspace, snapshot);
    }
    Point3 rightPoint(double distance)
    {
        const auto v = OrbitCamera{}.right() * distance;
        return {v.x(), v.y(), v.z()};
    }
    double rightPixel(double distance)
    {
        return 64 + distance * 96 / (20 * std::tan(std::numbers::pi / 6));
    }
    WorkspaceLayout layout{16, 16, 96, 96, 128, 128};
    WorkspaceInput input{64, 64, false, false, false, true, true, true, false, 0};
    std::unique_ptr<microsw::ApplicationWindow> window;
    std::unique_ptr<microsw::OpenGLContext> context;
};

TEST_F(WorkspaceSelectionTest, PointSegmentAndLinePressSelectRealIds)
{
    for (int kind = 0; kind < 3; ++kind)
    {
        GeometryPresentation presentation;
        if (kind == 0) (void)presentation.add(Point3{});
        if (kind == 1) (void)presentation.add(Segment3{rightPoint(-1), rightPoint(1)});
        if (kind == 2) (void)presentation.add(Line3{Point3{}, OrbitCamera{}.right()});
        WorkspaceViewport workspace{presentation};
        EXPECT_FALSE(workspace.selectedEntity());
        click(workspace, input);
        EXPECT_EQ(workspace.selectedEntity(), presentation.entities().front().id());
        EXPECT_EQ(presentation.size(), 1U);
    }
}

TEST_F(WorkspaceSelectionTest, HoverMovementAndNoHoverDoNotChangeSelection)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    const auto b = presentation.add(rightPoint(2));
    WorkspaceViewport workspace{presentation};
    frame(workspace, input);
    EXPECT_EQ(workspace.hoveredEntity(), a);
    EXPECT_FALSE(workspace.selectedEntity());
    click(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), a);
    input.x = rightPixel(2);
    frame(workspace, input);
    EXPECT_EQ(workspace.hoveredEntity(), b);
    EXPECT_EQ(workspace.selectedEntity(), a);
    input.y = 100;
    frame(workspace, input);
    EXPECT_FALSE(workspace.hoveredEntity());
    EXPECT_EQ(workspace.selectedEntity(), a);
    click(workspace, input);
    EXPECT_FALSE(workspace.selectedEntity());
}

TEST_F(WorkspaceSelectionTest, PressReplacesSelectionAndReclickDoesNotToggle)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    const auto b = presentation.add(rightPoint(2));
    WorkspaceViewport workspace{presentation};
    click(workspace, input);
    click(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), a);
    input.x = rightPixel(2);
    click(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), b);
}

TEST_F(WorkspaceSelectionTest, SelectionQueriesCurrentPointerWithoutDependingOnHover)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    const auto b = presentation.add(rightPoint(2));
    WorkspaceViewport workspace{presentation};
    frame(workspace, input);
    ASSERT_EQ(workspace.hoveredEntity(), a);
    input.x = rightPixel(2);
    input.leftPressed = true;
    workspace.updateSelection(layout, input, 128, 128);
    EXPECT_EQ(workspace.selectedEntity(), b);
    EXPECT_EQ(workspace.hoveredEntity(), a);
}

TEST_F(WorkspaceSelectionTest, FramesWithoutPressDoNotRepeatSelection)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    (void)presentation.add(rightPoint(2));
    WorkspaceViewport workspace{presentation};
    click(workspace, input);
    input.x = rightPixel(2);
    for (int frameIndex = 0; frameIndex < 3; ++frameIndex)
        frame(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), a);
    input.y = 100;
    frame(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), a);
}

TEST_F(WorkspaceSelectionTest, OutsideAndBlockedClicksPreserveSelectedIdentity)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    (void)presentation.add(rightPoint(2));
    WorkspaceViewport workspace{presentation};
    click(workspace, input);
    for (bool overEntity : {false, true})
        for (int gate = 0; gate < 7; ++gate)
        {
            auto blocked = input;
            blocked.x = rightPixel(2);
            if (!overEntity) blocked.y = 100;
            if (gate == 0) blocked.x = 15;
            if (gate == 1) blocked.x = 112;
            if (gate == 2) blocked.blocked = true;
            if (gate == 3) blocked.focused = false;
            if (gate == 4) blocked.pointerValid = false;
            if (gate == 5) blocked.workspaceHovered = false;
            if (gate == 6) blocked.x = std::numeric_limits<double>::quiet_NaN();
            click(workspace, blocked);
            EXPECT_EQ(workspace.selectedEntity(), a);
        }
}

TEST_F(WorkspaceSelectionTest, OrbitPanAndSimultaneousLeftPressPreserveSelection)
{
    for (bool shift : {false, true})
    {
        GeometryPresentation presentation;
        const auto a = presentation.add(Point3{});
        WorkspaceViewport workspace{presentation};
        click(workspace, input);
        auto gesture = input;
        gesture.shiftDown = shift;
        gesture.middlePressed = gesture.middleDown = gesture.leftPressed = true;
        frame(workspace, gesture);
        EXPECT_EQ(workspace.selectedEntity(), a);
        EXPECT_FALSE(workspace.hoveredEntity());
        gesture.middlePressed = false;
        gesture.x += 30;
        gesture.y += 20;
        frame(workspace, gesture);
        EXPECT_EQ(workspace.selectedEntity(), a);
        gesture.middleDown = gesture.leftPressed = false;
        frame(workspace, gesture);
        EXPECT_EQ(workspace.selectedEntity(), a);
    }
}

TEST_F(WorkspaceSelectionTest, ZoomAndProjectionChangesWithoutPressPreserveSelection)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(rightPoint(2));
    WorkspaceViewport workspace{presentation};
    input.x = rightPixel(2);
    click(workspace, input);
    for (auto mode : {ProjectionMode::Orthographic, ProjectionMode::Perspective})
    {
        input.projectionRequest = mode;
        input.wheelDelta = 10;
        frame(workspace, input);
        EXPECT_EQ(workspace.selectedEntity(), a);
    }
}

TEST_F(WorkspaceSelectionTest, ZoomAndPressUseResultingViewInSameFrame)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(rightPoint(2));
    const auto b = presentation.add(rightPoint(2 * std::exp(-0.75)));
    WorkspaceViewport workspace{presentation};
    input.x = rightPixel(2);
    click(workspace, input);
    ASSERT_EQ(workspace.selectedEntity(), a);
    input.wheelDelta = 5;
    click(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), b);
}

TEST_F(WorkspaceSelectionTest, ProjectionRequestAndPressUseResultingView)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(rightPoint(2));
    const auto b = presentation.add(rightPoint(2 * 10 / (20 * std::tan(std::numbers::pi / 6))));
    WorkspaceViewport workspace{presentation};
    input.x = rightPixel(2);
    click(workspace, input);
    ASSERT_EQ(workspace.selectedEntity(), a);
    input.projectionRequest = ProjectionMode::Orthographic;
    click(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), b);
    input.projectionRequest = ProjectionMode::Perspective;
    click(workspace, input);
    EXPECT_EQ(workspace.selectedEntity(), a);
}

TEST_F(WorkspaceSelectionTest, ResizeMinimizeAndInvalidSurfacesPreserveSelection)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    click(workspace, input);
    for (const auto& resized : {WorkspaceLayout{16,16,70,80,128,128}, WorkspaceLayout{},
                                WorkspaceLayout{16,16,0,96,128,128}})
    {
        workspace.updateSelection(resized, input, 128, 128);
        EXPECT_EQ(workspace.selectedEntity(), a);
    }
    input.leftPressed = true;
    for (int dimension : {0, -1})
    {
        EXPECT_NO_THROW(workspace.updateSelection(layout, input, dimension, 128));
        EXPECT_EQ(workspace.selectedEntity(), a);
    }
    EXPECT_NO_THROW(workspace.updateSelection({}, input, 128, 128));
    EXPECT_EQ(workspace.selectedEntity(), a);
}

TEST_F(WorkspaceSelectionTest, EffectiveRasterOutsideDoesNotClearButEmptyInsideDoes)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    click(workspace, input);
    const WorkspaceLayout fractional{16.25,16.25,96.5,96.5,128,128};
    input.leftPressed = true;
    // Inside UI layout, but outside its inward-rounded raster viewport.
    input.x = 16.5;
    input.y = 64;
    workspace.updateSelection(fractional, input, 128, 128);
    EXPECT_EQ(workspace.selectedEntity(), a);
    input.x = 18;
    workspace.updateSelection(fractional, input, 128, 128);
    EXPECT_FALSE(workspace.selectedEntity());
}

TEST_F(WorkspaceSelectionTest, HiDpiClickAdoptsExactPickerAndPreservesTieBreak)
{
    GeometryPresentation presentation;
    const auto first = presentation.add(Point3{});
    (void)presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    for (const auto& size : std::array{std::array{128,128}, std::array{256,192}, std::array{160,160}})
    {
        auto clickInput = input;
        clickInput.leftPressed = true;
        const auto hit = workspace.pick(layout, size[0], size[1], input.x, input.y);
        ASSERT_TRUE(hit);
        workspace.updateSelection(layout, clickInput, size[0], size[1]);
        EXPECT_EQ(workspace.selectedEntity(), hit->id);
        EXPECT_EQ(workspace.selectedEntity(), first);
    }
}

TEST_F(WorkspaceSelectionTest, EmptyAndDefaultWorkspacesAreSafe)
{
    const GeometryPresentation empty;
    WorkspaceViewport withEmpty{empty};
    WorkspaceViewport withoutPresentation;
    EXPECT_NO_THROW(click(withEmpty, input));
    EXPECT_NO_THROW(click(withoutPresentation, input));
    EXPECT_FALSE(withEmpty.selectedEntity());
    EXPECT_FALSE(withoutPresentation.selectedEntity());
}
}
