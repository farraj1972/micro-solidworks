#include "viewer/WorkspaceViewport.h"
#include "viewer/ReferenceGrid.h"
#include "viewer/ReferenceAxes.h"
#include "viewer/OrbitCamera.h"
#include "viewer/ViewProjection.h"
#include "rendering/LineRenderer.h"
#include "rendering/ShaderProgram.h"
#include "core/math/Tolerance.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <memory>

namespace
{
using microsw::WorkspaceLayout;
using microsw::viewer::framebufferRect;
using microsw::viewer::ViewportRect;
using microsw::viewer::WorkspaceViewport;

void expectRect(const ViewportRect& actual, int x, int y, int width, int height)
{
    EXPECT_EQ(actual.x, x);
    EXPECT_EQ(actual.y, y);
    EXPECT_EQ(actual.width, width);
    EXPECT_EQ(actual.height, height);
}

TEST(WorkspaceMapping, ConvertsTopLeftToBottomLeft)
{
    expectRect(framebufferRect({100, 50, 600, 400, 800, 600}, 800, 600), 100, 150, 600, 400);
}

TEST(WorkspaceMapping, UsesRealFramebufferScale)
{
    expectRect(framebufferRect({100, 50, 600, 400, 800, 600}, 1600, 1200), 200, 300, 1200, 800);
}

TEST(WorkspaceMapping, SupportsDifferentHorizontalAndVerticalScale)
{
    expectRect(framebufferRect({100, 50, 600, 400, 800, 600}, 1600, 600), 200, 150, 1200, 400);
}

TEST(WorkspaceMapping, ClipsToFramebuffer)
{
    expectRect(framebufferRect({-100, -50, 1000, 800, 800, 600}, 800, 600), 0, 0, 800, 600);
    expectRect(framebufferRect({700, 500, 200, 200, 800, 600}, 800, 600), 700, 0, 100, 100);
}

TEST(WorkspaceMapping, RejectsEmptyNegativeAndOffscreenRectangles)
{
    for (const WorkspaceLayout layout : {
        WorkspaceLayout{0, 0, 0, 100, 800, 600}, {0, 0, 100, 0, 800, 600},
        {0, 0, -1, 100, 800, 600}, {0, 0, 100, -1, 800, 600},
        {900, 0, 100, 100, 800, 600}, {0, 700, 100, 100, 800, 600},
        {0, 0, 100, 100, 0, 600}})
        expectRect(framebufferRect(layout, 800, 600), 0, 0, 0, 0);
}

TEST(WorkspaceMapping, MinimizedFramebufferIsEmpty)
{
    expectRect(framebufferRect({0, 0, 800, 600, 800, 600}, 0, 0), 0, 0, 0, 0);
    expectRect(framebufferRect({0, 0, 800, 600, 800, 600}, -1, 600), 0, 0, 0, 0);
}

TEST(WorkspaceMapping, NonFiniteLayoutIsEmpty)
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    expectRect(framebufferRect({nan, 0, 100, 100, 800, 600}, 800, 600), 0, 0, 0, 0);
    expectRect(framebufferRect({0, 0, inf, 100, 800, 600}, 800, 600), 0, 0, 0, 0);
    expectRect(framebufferRect({0, 0, 100, 100, 800, inf}, 800, 600), 0, 0, 0, 0);
}

TEST(WorkspaceMapping, FractionalBoundsRoundInward)
{
    expectRect(framebufferRect({10.25, 20.25, 100.5, 80.5, 200, 200}, 200, 200), 11, 100, 99, 79);
}

TEST(WorkspaceMapping, ResizeRecomputesAspect)
{
    const auto wide = framebufferRect({0, 0, 800, 400, 800, 400}, 800, 400);
    const auto tall = framebufferRect({0, 0, 400, 800, 400, 800}, 400, 800);
    EXPECT_TRUE(microsw::math::almostEqual(wide.aspectRatio(), 2.0));
    EXPECT_TRUE(microsw::math::almostEqual(tall.aspectRatio(), 0.5));
    EXPECT_TRUE(microsw::math::almostEqual(ViewportRect{}.aspectRatio(), 0.0));
}

class WorkspaceViewportTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<microsw::ApplicationWindow>(128, 128, "Workspace viewport tests");
        glfwHideWindow(static_cast<GLFWwindow*>(window->nativeHandle()));
        context = std::make_unique<microsw::OpenGLContext>(*window);
        ASSERT_EQ(glGetError(), GL_NO_ERROR);
    }
    void TearDown() override
    {
        if (context) EXPECT_EQ(glGetError(), GL_NO_ERROR);
        context.reset();
        window.reset();
    }
    std::unique_ptr<microsw::ApplicationWindow> window;
    std::unique_ptr<microsw::OpenGLContext> context;
};

TEST_F(WorkspaceViewportTest, ConstructsRendersResizesAndDestroys)
{
    {
        WorkspaceViewport viewport;
        viewport.render({16, 16, 96, 96, 128, 128}, 128, 128);
        viewport.render({16, 16, 64, 96, 128, 128}, 128, 128);
        viewport.render({0, 0, 1, 1, 128, 128}, 128, 128);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(WorkspaceViewportTest, GridAndAxesUploadAndDrawInOneDepthPass)
{
    const microsw::viewer::ReferenceGrid grid;
    const microsw::viewer::ReferenceAxes axes;
    std::array<microsw::rendering::LineRenderer, 4> batches;
    batches[0].setVertices(grid.vertices());
    batches[1].setVertices(axes.xAxis());
    batches[2].setVertices(axes.yAxis());
    batches[3].setVertices(axes.zAxis());
    EXPECT_EQ(batches[0].vertexCount(), 80u);
    ASSERT_EQ(glGetError(), GL_NO_ERROR);

    const char* vertex = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uMVP;
void main() { gl_Position = uMVP * vec4(aPosition, 1.0); }
)";
    const char* fragment = R"(#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main() { FragColor = vec4(uColor, 1.0); }
)";
    microsw::rendering::ShaderProgram shader{vertex, fragment};
    shader.bind();
    shader.setMatrix4("uMVP", microsw::viewer::perspective(1.0, 1.0, 0.1, 100.0)
        * microsw::viewer::viewMatrix(microsw::viewer::OrbitCamera{}));
    const std::array<microsw::math::Vector3, 4> colors{
        microsw::math::Vector3{0.35, 0.35, 0.38}, microsw::math::Vector3{1.0, 0.0, 0.0},
        microsw::math::Vector3{0.0, 1.0, 0.0}, microsw::math::Vector3{0.0, 0.0, 1.0}};
    glViewport(0, 0, 128, 128);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (std::size_t i = 0; i < batches.size(); ++i)
    {
        SCOPED_TRACE(i);
        shader.setVector3("uColor", colors[i]);
        batches[i].draw();
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
        EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_TRUE);
        GLint depthFunction{};
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
        EXPECT_EQ(depthFunction, GL_LESS);
    }
}

TEST_F(WorkspaceViewportTest, PanZoomAndOrbitRenderWithoutErrors)
{
    WorkspaceViewport viewport;
    const WorkspaceLayout layout{16, 16, 96, 96, 128, 128};
    microsw::WorkspaceInput input{40, 40, true, true, true, true, true, true, false, 0};
    viewport.updateNavigation(layout, input);
    input.middlePressed = false;
    input.x = 60;
    viewport.updateNavigation(layout, input);
    viewport.render(layout, 128, 128);
    input.middleDown = false;
    input.wheelDelta = 10000;
    viewport.updateNavigation(layout, input);
    viewport.render(layout, 128, 128);
    input.wheelDelta = -10000;
    viewport.updateNavigation(layout, input);
    viewport.render(layout, 128, 128);
    input.wheelDelta = 0;
    input.shiftDown = false;
    input.middlePressed = input.middleDown = true;
    viewport.updateNavigation(layout, input);
    input.middlePressed = false;
    input.y = 70;
    viewport.updateNavigation(layout, input);
    viewport.render(layout, 128, 128);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(WorkspaceViewportTest, EmptyRenderDoesNotChangeState)
{
    WorkspaceViewport viewport;
    glViewport(1, 2, 3, 4);
    viewport.render({}, 128, 128);
    viewport.render({0, 0, 128, 128, 128, 128}, 0, 0);
    GLint actual[4]{};
    glGetIntegerv(GL_VIEWPORT, actual);
    for (int i = 0; i < 4; ++i) EXPECT_EQ(actual[i], i + 1);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(WorkspaceViewportTest, RenderRestoresPassState)
{
    WorkspaceViewport viewport;
    glViewport(1, 2, 30, 40);
    glScissor(5, 6, 70, 80);
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_GREATER);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
    glClearColor(0.25F, 0.5F, 0.75F, 1.0F);
    glClearDepth(0.5);
    viewport.render({16, 16, 96, 96, 128, 128}, 128, 128);

    GLint actual[4]{}, depthFunc{}, program{}, vao{};
    glGetIntegerv(GL_VIEWPORT, actual);
    const std::array<int, 4> expectedViewport{1, 2, 30, 40};
    for (int i = 0; i < 4; ++i) EXPECT_EQ(actual[i], expectedViewport[i]);
    glGetIntegerv(GL_SCISSOR_BOX, actual);
    const std::array<int, 4> expectedScissor{5, 6, 70, 80};
    for (int i = 0; i < 4; ++i) EXPECT_EQ(actual[i], expectedScissor[i]);
    EXPECT_EQ(glIsEnabled(GL_SCISSOR_TEST), GL_TRUE);
    EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_FALSE);
    EXPECT_EQ(glIsEnabled(GL_BLEND), GL_TRUE);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    EXPECT_EQ(depthFunc, GL_GREATER);
    GLboolean mask{}, colors[4]{};
    glGetBooleanv(GL_DEPTH_WRITEMASK, &mask);
    EXPECT_EQ(mask, GL_FALSE);
    glGetBooleanv(GL_COLOR_WRITEMASK, colors);
    EXPECT_EQ(colors[0], GL_TRUE);
    EXPECT_EQ(colors[1], GL_FALSE);
    EXPECT_EQ(colors[2], GL_TRUE);
    EXPECT_EQ(colors[3], GL_FALSE);
    GLfloat color[4]{};
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    EXPECT_FLOAT_EQ(color[0], 0.25F);
    EXPECT_FLOAT_EQ(color[1], 0.5F);
    EXPECT_FLOAT_EQ(color[2], 0.75F);
    GLdouble depth{};
    glGetDoublev(GL_DEPTH_CLEAR_VALUE, &depth);
    EXPECT_DOUBLE_EQ(depth, 0.5);
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    EXPECT_EQ(program, 0);
    EXPECT_EQ(vao, 0);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}
}
