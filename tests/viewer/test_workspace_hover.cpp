#include "viewer/WorkspaceViewport.h"
#include "viewer/OrbitCamera.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "presentation/GeometryPresentation.h"
#include "app/demo/GeometryDemoScene.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <numbers>
#include <limits>
#include <memory>

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

class WorkspaceHoverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<microsw::ApplicationWindow>(128, 128, "Hover tests");
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
    }
    Point3 point(const microsw::math::Vector3& v) { return {v.x(), v.y(), v.z()}; }
    WorkspaceLayout layout{16, 16, 96, 96, 128, 128};
    WorkspaceInput input{64, 64, false, false, false, true, true, true, false, 0};
    std::unique_ptr<microsw::ApplicationWindow> window;
    std::unique_ptr<microsw::OpenGLContext> context;
};

TEST_F(WorkspaceHoverTest, PointSegmentAndLineAdoptExistingPickerIdentity)
{
    const OrbitCamera camera;
    for (int kind = 0; kind < 3; ++kind)
    {
        GeometryPresentation presentation;
        if (kind == 0) (void)presentation.add(Point3{});
        if (kind == 1) (void)presentation.add(Segment3{point(-camera.right()), point(camera.right())});
        if (kind == 2) (void)presentation.add(Line3{Point3{}, camera.right()});
        WorkspaceViewport workspace{presentation};
        input.y = 67; // Three logical pixels from each projected primitive.
        frame(workspace, input);
        const auto picked = workspace.pick(layout, 128, 128, input.x, input.y);
        ASSERT_TRUE(picked);
        EXPECT_EQ(workspace.hoveredEntity(), picked->id);
        EXPECT_EQ(workspace.hoveredEntity(), presentation.entities().front().id());
    }
}

TEST_F(WorkspaceHoverTest, MovingBetweenCandidatesAndNoHitReplacesOrClearsHover)
{
    const OrbitCamera camera;
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{});
    const auto b = presentation.add(point(camera.right() * 2));
    WorkspaceViewport workspace{presentation};
    frame(workspace, input);
    EXPECT_EQ(workspace.hoveredEntity(), a);
    input.x = 64 + 2 * 96 / (20 * std::tan(std::numbers::pi / 6));
    frame(workspace, input);
    EXPECT_EQ(workspace.hoveredEntity(), b);
    input.y = 100;
    frame(workspace, input);
    EXPECT_FALSE(workspace.hoveredEntity());
}

TEST_F(WorkspaceHoverTest, InteractionGatesClearAndReentryRecalculates)
{
    GeometryPresentation presentation;
    const auto id = presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    for (int gate = 0; gate < 6; ++gate)
    {
        frame(workspace, input);
        ASSERT_EQ(workspace.hoveredEntity(), id);
        auto blocked = input;
        if (gate == 0) blocked.x = 15;
        if (gate == 1) blocked.focused = false;
        if (gate == 2) blocked.pointerValid = false;
        if (gate == 3) blocked.workspaceHovered = false;
        if (gate == 4) blocked.blocked = true; // Existing UI capture / modal flag.
        if (gate == 5) blocked.x = std::numeric_limits<double>::quiet_NaN();
        frame(workspace, blocked);
        EXPECT_FALSE(workspace.hoveredEntity());
        frame(workspace, input);
        EXPECT_EQ(workspace.hoveredEntity(), id);
    }
}

TEST_F(WorkspaceHoverTest, OrbitAndPanSuspendUntilMmbRelease)
{
    for (bool shift : {false, true})
    {
        GeometryPresentation presentation;
        const auto id = presentation.add(Point3{});
        WorkspaceViewport workspace{presentation};
        frame(workspace, input);
        ASSERT_EQ(workspace.hoveredEntity(), id);
        auto gesture = input;
        gesture.middlePressed = gesture.middleDown = true;
        gesture.shiftDown = shift;
        frame(workspace, gesture);
        EXPECT_FALSE(workspace.hoveredEntity());
        gesture.middlePressed = false;
        frame(workspace, gesture);
        EXPECT_FALSE(workspace.hoveredEntity());
        frame(workspace, input);
        EXPECT_EQ(workspace.hoveredEntity(), id);
    }
}

TEST_F(WorkspaceHoverTest, MovingNavigationReleasesIntoCurrentViewWithoutStaleHover)
{
    const OrbitCamera camera;
    for (bool shift : {false, true})
    {
        GeometryPresentation presentation;
        (void)presentation.add(point(camera.right() * 2));
        WorkspaceViewport workspace{presentation};
        auto cursor = input;
        cursor.x = 64 + 2 * 96 / (20 * std::tan(std::numbers::pi / 6));
        frame(workspace, cursor);
        ASSERT_TRUE(workspace.hoveredEntity());
        auto gesture = cursor;
        gesture.middlePressed = gesture.middleDown = true;
        gesture.shiftDown = shift;
        frame(workspace, gesture);
        gesture.middlePressed = false;
        gesture.x += 25;
        gesture.y += 15;
        frame(workspace, gesture);
        EXPECT_FALSE(workspace.hoveredEntity());
        frame(workspace, cursor);
        const auto picked = workspace.pick(layout, 128, 128, cursor.x, cursor.y);
        EXPECT_EQ(workspace.hoveredEntity(),
            picked ? std::optional{picked->id} : std::nullopt);
    }
}

TEST_F(WorkspaceHoverTest, ZoomUsesUpdatedViewInSameFrameForBothModes)
{
    const OrbitCamera camera;
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        GeometryPresentation presentation;
        const auto id = presentation.add(point(camera.right() * 2));
        WorkspaceViewport workspace{presentation};
        workspace.setProjectionMode(mode);
        auto cursor = input;
        const auto height = mode == ProjectionMode::Perspective
            ? 20 * std::tan(std::numbers::pi / 6) : 10;
        cursor.x = 64 + 2 * 96 / height;
        frame(workspace, cursor);
        ASSERT_EQ(workspace.hoveredEntity(), id);
        cursor.wheelDelta = 5;
        frame(workspace, cursor);
        EXPECT_FALSE(workspace.hoveredEntity());
        cursor.wheelDelta = -5;
        frame(workspace, cursor);
        EXPECT_EQ(workspace.hoveredEntity(), id);
    }
}

TEST_F(WorkspaceHoverTest, ProjectionCommandsRecomputeAtStationaryCursor)
{
    const OrbitCamera camera;
    GeometryPresentation presentation;
    const auto id = presentation.add(point(camera.right() * 2));
    WorkspaceViewport workspace{presentation};
    auto cursor = input;
    cursor.x = 64 + 2 * 96 / (20 * std::tan(std::numbers::pi / 6)) - 5;
    frame(workspace, cursor);
    ASSERT_EQ(workspace.hoveredEntity(), id);
    cursor.projectionRequest = ProjectionMode::Orthographic;
    frame(workspace, cursor);
    EXPECT_FALSE(workspace.hoveredEntity());
    cursor.projectionRequest = ProjectionMode::Perspective;
    frame(workspace, cursor);
    EXPECT_EQ(workspace.hoveredEntity(), id);
}

TEST_F(WorkspaceHoverTest, DegenerateResizeAndMinimizeClearWithoutThrowing)
{
    GeometryPresentation presentation;
    (void)presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    for (const auto& invalid : {WorkspaceLayout{}, WorkspaceLayout{16,16,0,96,128,128},
                                WorkspaceLayout{16,16,96,-1,128,128},
                                WorkspaceLayout{16,16,96,96,0,128}})
    {
        frame(workspace, input);
        ASSERT_TRUE(workspace.hoveredEntity());
        EXPECT_NO_THROW(workspace.updateHover(invalid, input, 128, 128));
        EXPECT_FALSE(workspace.hoveredEntity());
    }
    for (int dimension : {0, -1})
    {
        frame(workspace, input);
        EXPECT_NO_THROW(workspace.updateHover(layout, input, dimension, 128));
        EXPECT_FALSE(workspace.hoveredEntity());
    }
    frame(workspace, input);
    EXPECT_TRUE(workspace.hoveredEntity());
}

TEST_F(WorkspaceHoverTest, ResizeAndHiDpiReuseEffectiveRasterPicking)
{
    const OrbitCamera camera;
    GeometryPresentation presentation;
    (void)presentation.add(point(camera.right()));
    WorkspaceViewport workspace{presentation};
    for (const auto& size : std::array{std::array{128,128}, std::array{256,192}, std::array{160,160}})
        for (double width : {96.0, 70.5})
        {
            auto resized = layout;
            resized.x = 16.25;
            resized.width = width;
            auto cursor = input;
            const auto rect = microsw::viewer::framebufferRect(resized, size[0], size[1]);
            const auto logicalWidth = rect.width * resized.displayWidth / size[0];
            cursor.x = rect.x * resized.displayWidth / size[0] + logicalWidth / 2
                + logicalWidth / (20 * std::tan(std::numbers::pi / 6) * rect.aspectRatio());
            workspace.updateHover(resized, cursor, size[0], size[1]);
            const auto picked = workspace.pick(resized, size[0], size[1], cursor.x, cursor.y);
            ASSERT_TRUE(picked);
            EXPECT_EQ(workspace.hoveredEntity(), picked->id);
        }
}

TEST_F(WorkspaceHoverTest, TieBreakRemainsOwnedByPicker)
{
    GeometryPresentation presentation;
    const auto first = presentation.add(Point3{});
    (void)presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        frame(workspace, input);
        EXPECT_EQ(workspace.hoveredEntity(), first);
    }
}

TEST_F(WorkspaceHoverTest, EmptyAndDefaultWorkspacesHaveNoHover)
{
    const GeometryPresentation empty;
    WorkspaceViewport withEmpty{empty};
    WorkspaceViewport withoutPresentation;
    frame(withEmpty, input);
    frame(withoutPresentation, input);
    EXPECT_FALSE(withEmpty.hoveredEntity());
    EXPECT_FALSE(withoutPresentation.hoveredEntity());
}

TEST_F(WorkspaceHoverTest, RealDemoHoverDoesNotMutateGeometryOrRenderState)
{
    const auto demo = microsw::demo::createGeometryDemoScene();
    WorkspaceViewport workspace{demo};
    glDisable(GL_DEPTH_TEST);
    frame(workspace, input);
    ASSERT_TRUE(workspace.hoveredEntity());
    EXPECT_EQ(workspace.hoveredEntity(), demo.entities().front().id());
    EXPECT_EQ(demo.size(), 8U);
    EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_FALSE);
    workspace.render(layout, 128, 128);
    EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_FALSE);
    EXPECT_EQ(demo.entities().front().id(), *workspace.hoveredEntity());
}
}
