#include "viewer/WorkspaceViewport.h"
#include "viewer/VisualState.h"
#include "presentation/GeometryPresentation.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include <numbers>

namespace
{
using namespace microsw;
using namespace microsw::viewer;
class WorkspaceHighlightTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<ApplicationWindow>(128, 128, "Highlight tests");
        glfwHideWindow(static_cast<GLFWwindow*>(window->nativeHandle()));
        context = std::make_unique<OpenGLContext>(*window);
    }
    void TearDown() override
    {
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
        context.reset();
        window.reset();
    }
    geometry::Point3 point(double right, double up)
    {
        const auto v = OrbitCamera{}.right() * right + OrbitCamera{}.up() * up;
        return {v.x(), v.y(), v.z()};
    }
    void frame(WorkspaceViewport& workspace, WorkspaceInput input)
    {
        workspace.updateNavigation(layout, input);
        workspace.updateHover(layout, input, 128, 128);
        workspace.updateSelection(layout, input, 128, 128);
        workspace.render(layout, 128, 128);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
    const WorkspaceLayout layout{16, 16, 96, 96, 128, 128};
    WorkspaceInput input{64, 64, false, false, false, true, true, true, false, 0};
    std::unique_ptr<ApplicationWindow> window;
    std::unique_ptr<OpenGLContext> context;
};

TEST_F(WorkspaceHighlightTest, AllPrimitiveHighlightsCoexistWithNormalGeometryGridAndAxes)
{
    for (int kind = 0; kind < 3; ++kind)
    {
        presentation::GeometryPresentation presentation;
        const auto selected = kind == 0 ? presentation.add(point(0, 0))
            : kind == 1 ? presentation.add(geometry::Segment3{point(-1, 0), point(1, 0)})
            : presentation.add(geometry::Line3{point(0, 0), OrbitCamera{}.right()});
        const auto hovered = presentation.add(point(2, 2));
        (void)presentation.add(geometry::Segment3{point(-2, -2), point(2, -2)});
        (void)presentation.add(geometry::Line3{point(0, -3), OrbitCamera{}.right()});
        (void)presentation.add(point(-2, 2));
        WorkspaceViewport workspace{presentation};
        frame(workspace, input);
        EXPECT_EQ(visualStateFor(selected, workspace.hoveredEntity(), workspace.selectedEntity()),
            VisualState::Hovered);
        auto click = input;
        click.leftPressed = true;
        frame(workspace, click);
        EXPECT_EQ(visualStateFor(selected, workspace.hoveredEntity(), workspace.selectedEntity()),
            VisualState::Selected);
        const double offset = 2 * 96 / (20 * std::tan(std::numbers::pi / 6));
        auto moved = input;
        moved.x += offset;
        moved.y -= offset;
        frame(workspace, moved);
        EXPECT_EQ(workspace.hoveredEntity(), hovered);
        EXPECT_EQ(workspace.selectedEntity(), selected);
        // The highlight pass must restore external depth state too.
        glDepthFunc(GL_GREATER);
        glDisable(GL_DEPTH_TEST);
        workspace.render(layout, 128, 128);
        GLint depth{};
        glGetIntegerv(GL_DEPTH_FUNC, &depth);
        EXPECT_EQ(depth, GL_GREATER);
        EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_FALSE);
        moved.x = 105;
        moved.y = 20;
        moved.leftPressed = true;
        frame(workspace, moved);
        EXPECT_FALSE(workspace.selectedEntity());
        EXPECT_FALSE(workspace.hoveredEntity());
        EXPECT_EQ(presentation.size(), 5U);
    }
}

TEST_F(WorkspaceHighlightTest, HighlightSurvivesNavigationProjectionAndHiDpiRestore)
{
    presentation::GeometryPresentation presentation;
    const auto id = presentation.add(point(0, 0));
    WorkspaceViewport workspace{presentation};
    auto click = input;
    click.leftPressed = true;
    frame(workspace, click);
    for (bool pan : {false, true})
    {
        auto drag = input;
        drag.shiftDown = pan;
        drag.middlePressed = drag.middleDown = true;
        frame(workspace, drag);
        drag.middlePressed = false;
        drag.x += 20;
        frame(workspace, drag);
        EXPECT_FALSE(workspace.hoveredEntity());
        EXPECT_EQ(workspace.selectedEntity(), id);
        frame(workspace, input);
    }
    for (auto mode : {ProjectionMode::Orthographic, ProjectionMode::Perspective})
    {
        auto zoom = input;
        zoom.projectionRequest = mode;
        zoom.wheelDelta = 2;
        frame(workspace, zoom);
        workspace.render({}, 0, 0);
        workspace.render({8.25, 8.25, 110.5, 90.5, 128, 128}, 256, 192);
        workspace.render(layout, 128, 128);
        EXPECT_EQ(workspace.selectedEntity(), id);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
}
}
