#include "app/window/ApplicationWindow.h"
#include "core/math/Transformations.h"
#include "viewer/OrbitNavigation.h"
#include "viewer/PanZoomNavigation.h"
#include "viewer/ProjectionState.h"
#include "viewer/ReferenceAxes.h"
#include "viewer/ReferenceGrid.h"
#include "viewer/ViewProjection.h"
#include "viewer/WorkspaceViewport.h"
#include "rendering/OpenGLContext.h"
#include "rendering/ShaderProgram.h"
#include "rendering/LineRenderer.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <memory>
#include <numbers>

namespace
{
using namespace microsw::math;
using namespace microsw::viewer;
using microsw::ProjectionMode;
using microsw::WorkspaceInput;
using microsw::WorkspaceLayout;

constexpr WorkspaceLayout layout{16, 16, 96, 96, 128, 128};
constexpr double fov = std::numbers::pi / 3.0;
constexpr double nearPlane = 0.1;
constexpr double farPlane = 1100.0;
constexpr std::array modes{ProjectionMode::Perspective, ProjectionMode::Orthographic};

WorkspaceInput pointer()
{
    return {48, 48, false, false, false, true, true, true, false, 0};
}

// Input sequencing only; all navigation and mathematics are production code.
struct Navigation
{
    OrbitCamera camera;
    ProjectionState projection;
    OrbitNavigation orbit;
    PanZoomNavigation panZoom;

    void handle(const WorkspaceInput& input)
    {
        orbit.handle(layout, input, camera);
        panZoom.handle(layout, input, camera, projection);
    }

    void drag(bool pan, double dx = 20, double dy = -10)
    {
        auto input = pointer();
        input.shiftDown = pan;
        input.middlePressed = input.middleDown = true;
        handle(input);
        input.middlePressed = false;
        input.x += dx;
        input.y += dy;
        handle(input);
        input.middleDown = false;
        handle(input);
    }

    void wheel(double delta)
    {
        auto input = pointer();
        input.wheelDelta = delta;
        handle(input);
    }
};

void expectPose(const OrbitCamera& actual, const OrbitCamera& expected)
{
    EXPECT_TRUE(almostEqual(actual.target(), expected.target()));
    EXPECT_TRUE(almostEqual(actual.yaw(), expected.yaw()));
    EXPECT_TRUE(almostEqual(actual.pitch(), expected.pitch()));
    EXPECT_TRUE(almostEqual(actual.distance(), expected.distance()));
}

void expectFinite(const Matrix4& matrix)
{
    for (std::size_t row = 0; row < 4; ++row)
        for (std::size_t column = 0; column < 4; ++column)
            EXPECT_TRUE(std::isfinite(matrix(row, column)));
}

void expectView(const OrbitCamera& camera)
{
    const auto view = viewMatrix(camera);
    expectFinite(view);
    EXPECT_TRUE(almostEqual(transformPoint(view, camera.position()), Vector3{}));
    EXPECT_TRUE(almostEqual(transformDirection(view, camera.forward()), Vector3{0, 0, -1}));
    EXPECT_TRUE(almostEqual(transformPoint(view, camera.target()), Vector3{0, 0, -camera.distance()}));
    for (const auto basis : {camera.forward(), camera.right(), camera.up()})
    {
        EXPECT_TRUE(std::isfinite(basis.x()));
        EXPECT_TRUE(std::isfinite(basis.y()));
        EXPECT_TRUE(std::isfinite(basis.z()));
        EXPECT_TRUE(almostEqual(basis.length(), 1.0));
    }
}

// Test-only homogeneous application, not a second projection implementation.
// The affine production helpers intentionally do not perform perspective division.
Vector3 ndc(const Matrix4& matrix, const Vector3& point)
{
    const std::array<double, 4> input{point.x(), point.y(), point.z(), 1};
    std::array<double, 4> clip{};
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
            clip[row] += matrix(row, column) * input[column];
        EXPECT_TRUE(std::isfinite(clip[row]));
    }
    EXPECT_GT(clip[3], 0);
    if (!std::isfinite(clip[3]) || clip[3] <= 0) return {};
    const Vector3 result{clip[0] / clip[3], clip[1] / clip[3], clip[2] / clip[3]};
    EXPECT_TRUE(std::isfinite(result.x()));
    EXPECT_TRUE(std::isfinite(result.y()));
    EXPECT_TRUE(std::isfinite(result.z()));
    return result;
}

void expectProjectedTarget(const Navigation& navigation)
{
    const auto view = viewMatrix(navigation.camera);
    const auto projection = navigation.projection.matrix(fov, 1.5, nearPlane, farPlane);
    expectFinite(projection);
    EXPECT_LT(transformPoint(view, navigation.camera.target()).z(), 0);
    const auto result = ndc(projection * view, navigation.camera.target());
    EXPECT_TRUE(almostEqual(result.x(), 0.0));
    EXPECT_TRUE(almostEqual(result.y(), 0.0));
    EXPECT_GE(result.z(), -1.0 - defaultAbsoluteTolerance);
    EXPECT_LE(result.z(), 1.0 + defaultAbsoluteTolerance);
}

TEST(ViewerIntegration, OrbitPreservesViewConventions)
{
    Navigation navigation;
    const auto before = navigation.camera;
    navigation.drag(false);
    EXPECT_GT(navigation.camera.yaw(), before.yaw());
    EXPECT_GT(navigation.camera.pitch(), before.pitch());
    EXPECT_TRUE(almostEqual(navigation.camera.target(), before.target()));
    EXPECT_TRUE(almostEqual(navigation.camera.distance(), before.distance()));
    expectView(navigation.camera);
}

TEST(ViewerIntegration, PanAfterOrbitTranslatesEyeAndTargetTogetherInBothModes)
{
    for (auto mode : modes)
    {
        Navigation navigation;
        navigation.projection.setMode(mode);
        navigation.drag(false);
        const auto before = navigation.camera;
        navigation.drag(true);
        const auto delta = navigation.camera.target() - before.target();
        EXPECT_GT(delta.length(), 0);
        EXPECT_TRUE(almostEqual(navigation.camera.position() - before.position(), delta));
        EXPECT_TRUE(almostEqual(dot(delta, before.forward()), 0.0));
        EXPECT_TRUE(almostEqual(navigation.camera.yaw(), before.yaw()));
        EXPECT_TRUE(almostEqual(navigation.camera.pitch(), before.pitch()));
        EXPECT_TRUE(almostEqual(navigation.camera.distance(), before.distance()));
        expectView(navigation.camera);
    }
}

TEST(ViewerIntegration, PerspectiveZoomAfterOrbitAndPanKeepsTargetInFront)
{
    Navigation navigation;
    navigation.drag(false);
    navigation.drag(true);
    const auto before = navigation.camera;
    navigation.wheel(2);
    EXPECT_LT(navigation.camera.distance(), before.distance());
    EXPECT_TRUE(almostEqual(navigation.camera.target(), before.target()));
    EXPECT_TRUE(almostEqual(navigation.camera.yaw(), before.yaw()));
    EXPECT_TRUE(almostEqual(navigation.camera.pitch(), before.pitch()));
    expectView(navigation.camera);
    expectProjectedTarget(navigation);
}

TEST(ViewerIntegration, OrthographicZoomAfterNavigationChangesOnlyProjectionScale)
{
    Navigation navigation;
    navigation.projection.setMode(ProjectionMode::Orthographic);
    navigation.drag(false);
    navigation.drag(true);
    const auto before = navigation.camera;
    const auto oldMatrix = navigation.projection.matrix(fov, 1.5, nearPlane, farPlane);
    const auto height = navigation.projection.visibleHeight();
    navigation.wheel(2);
    expectPose(navigation.camera, before);
    EXPECT_LT(navigation.projection.visibleHeight(), height);
    const auto newMatrix = navigation.projection.matrix(fov, 1.5, nearPlane, farPlane);
    EXPECT_GT(newMatrix(0, 0), oldMatrix(0, 0));
    EXPECT_GT(newMatrix(1, 1), oldMatrix(1, 1));
    expectProjectedTarget(navigation);
}

TEST(ViewerIntegration, MixedNavigationRoundTripPreservesIndependentZoomAndCurrentPose)
{
    Navigation navigation;
    navigation.wheel(-2);
    navigation.drag(false);
    navigation.drag(true);
    const auto perspectivePose = navigation.camera;
    const auto distance = navigation.camera.distance();
    navigation.projection.setMode(ProjectionMode::Orthographic);
    expectPose(navigation.camera, perspectivePose);
    navigation.wheel(3);
    navigation.drag(false, -30, 15);
    navigation.drag(true, 12, 8);
    const auto orthographicPose = navigation.camera;
    const auto height = navigation.projection.visibleHeight();
    EXPECT_TRUE(almostEqual(navigation.camera.distance(), distance));
    EXPECT_FALSE(almostEqual(orthographicPose.target(), perspectivePose.target()));
    EXPECT_FALSE(almostEqual(orthographicPose.yaw(), perspectivePose.yaw()));
    navigation.projection.setMode(ProjectionMode::Perspective);
    expectPose(navigation.camera, orthographicPose);
    EXPECT_TRUE(almostEqual(navigation.camera.distance(), distance));
    EXPECT_TRUE(almostEqual(navigation.projection.visibleHeight(), height));
    expectProjectedTarget(navigation);
    navigation.projection.setMode(ProjectionMode::Orthographic);
    expectPose(navigation.camera, orthographicPose);
    EXPECT_TRUE(almostEqual(navigation.projection.visibleHeight(), height));
    expectProjectedTarget(navigation);
}

TEST(ViewerIntegration, NavigatedWorldDepthMapsToBothNdcEndpoints)
{
    Navigation navigation;
    navigation.drag(false);
    navigation.drag(true);
    navigation.wheel(1);
    for (auto mode : modes)
    {
        navigation.projection.setMode(mode);
        const auto matrix = navigation.projection.matrix(fov, 1.5, nearPlane, farPlane)
            * viewMatrix(navigation.camera);
        const auto eye = navigation.camera.position();
        const auto forward = navigation.camera.forward();
        EXPECT_TRUE(almostEqual(ndc(matrix, eye + forward * nearPlane), Vector3{0, 0, -1}));
        EXPECT_TRUE(almostEqual(ndc(matrix, eye + forward * farPlane), Vector3{0, 0, 1}));
    }
}

TEST(ViewerIntegration, FramebufferMappingDrivesAspectWithoutMutatingNavigationState)
{
    Navigation navigation;
    navigation.drag(false);
    navigation.drag(true);
    navigation.wheel(-1);
    navigation.projection.setVisibleHeight(7);
    const auto before = navigation.camera;
    struct Mapping { WorkspaceLayout layout; int width, height; ViewportRect expected; };
    const std::array mappings{
        Mapping{{16, 8, 96, 64, 128, 128}, 128, 128, {16, 56, 96, 64}},
        Mapping{{16, 8, 96, 64, 128, 128}, 256, 256, {32, 112, 192, 128}},
        Mapping{{16, 8, 96, 64, 128, 128}, 256, 128, {32, 56, 192, 64}},
        Mapping{{-16, 8, 64, 64, 128, 128}, 128, 128, {0, 56, 48, 64}}};
    for (auto mode : modes)
    {
        navigation.projection.setMode(mode);
        const auto original = navigation.projection.matrix(fov, 1.5, nearPlane, farPlane);
        for (const auto& mapping : mappings)
        {
            const auto rect = framebufferRect(mapping.layout, mapping.width, mapping.height);
            EXPECT_EQ(rect.x, mapping.expected.x);
            EXPECT_EQ(rect.y, mapping.expected.y);
            EXPECT_EQ(rect.width, mapping.expected.width);
            EXPECT_EQ(rect.height, mapping.expected.height);
            const auto aspect = mapping.expected.aspectRatio();
            EXPECT_TRUE(almostEqual(rect.aspectRatio(), aspect));
            const auto projection = navigation.projection.matrix(fov, rect.aspectRatio(), nearPlane, farPlane);
            EXPECT_TRUE(almostEqual(projection(0, 0) * aspect, original(0, 0) * 1.5));
            EXPECT_TRUE(almostEqual(projection(1, 1), original(1, 1)));
            if (!almostEqual(aspect, 1.5)) EXPECT_FALSE(almostEqual(projection, original));
            expectFinite(projection);
            expectPose(navigation.camera, before);
            EXPECT_EQ(navigation.projection.mode(), mode);
            EXPECT_TRUE(almostEqual(navigation.projection.visibleHeight(), 7.0));
        }
    }
}

TEST(ViewerIntegration, HeldButtonCannotConvertOrbitAndPanInEitherProjection)
{
    for (auto mode : modes)
    {
        for (bool startPan : {false, true})
        {
            Navigation navigation;
            navigation.projection.setMode(mode);
            auto input = pointer();
            input.middlePressed = input.middleDown = true;
            input.shiftDown = startPan;
            navigation.handle(input);
            input.middlePressed = false;
            input.x += 10;
            navigation.handle(input);
            const auto before = navigation.camera;
            input.shiftDown = !startPan;
            input.x += 10;
            navigation.handle(input);
            EXPECT_FALSE(navigation.orbit.active());
            EXPECT_FALSE(navigation.panZoom.active());
            expectPose(navigation.camera, before);
            input.x += 10;
            navigation.handle(input);
            expectPose(navigation.camera, before);
            input.middleDown = false;
            navigation.handle(input);
            input.middlePressed = input.middleDown = true;
            navigation.handle(input);
            EXPECT_EQ(navigation.orbit.active(), startPan);
            EXPECT_EQ(navigation.panZoom.active(), !startPan);
            expectPose(navigation.camera, before); // Fresh press anchors, never jumps.
            input.middlePressed = false;
            input.y += 10;
            navigation.handle(input);
            EXPECT_FALSE(almostEqual(navigation.camera.position(), before.position()));
            expectView(navigation.camera);
        }
    }
}

TEST(ViewerIntegration, BlockingFocusAndPointerFailureCancelBothGesturesAndWheel)
{
    for (auto mode : modes)
        for (bool pan : {false, true})
            for (int reason = 0; reason < 3; ++reason)
            {
                SCOPED_TRACE(reason);
                Navigation navigation;
                navigation.projection.setMode(mode);
                auto input = pointer();
                input.middlePressed = input.middleDown = true;
                input.shiftDown = pan;
                navigation.handle(input);
                input.middlePressed = false;
                input.x += 10;
                navigation.handle(input);
                const auto before = navigation.camera;
                const auto height = navigation.projection.visibleHeight();
                input.x += 20;
                input.wheelDelta = 2;
                if (reason == 0) input.blocked = true;
                if (reason == 1) input.focused = false;
                if (reason == 2) input.pointerValid = false;
                navigation.handle(input);
                EXPECT_FALSE(navigation.orbit.active());
                EXPECT_FALSE(navigation.panZoom.active());
                expectPose(navigation.camera, before);
                EXPECT_TRUE(almostEqual(navigation.projection.visibleHeight(), height));
                input.blocked = false;
                input.focused = input.pointerValid = true;
                input.wheelDelta = 0;
                input.x += 10;
                navigation.handle(input);
                EXPECT_FALSE(navigation.orbit.active());
                EXPECT_FALSE(navigation.panZoom.active());
                expectPose(navigation.camera, before);
                expectView(navigation.camera);
            }
}

class ViewerIntegrationGL : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<microsw::ApplicationWindow>(256, 256, "Viewer integration");
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

// Test shader fixture exercises the established production uniform contract.
constexpr const char* vertex = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main() { gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0); }
)";
constexpr const char* fragment = R"(#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main() { FragColor = vec4(uColor, 1.0); }
)";

struct AidPass
{
    ReferenceGrid grid;
    ReferenceAxes axes;
    microsw::rendering::ShaderProgram shader{vertex, fragment};
    std::array<microsw::rendering::LineRenderer, 4> lines;

    AidPass()
    {
        lines[0].setVertices(grid.vertices());
        lines[1].setVertices(axes.xAxis());
        lines[2].setVertices(axes.yAxis());
        lines[3].setVertices(axes.zAxis());
    }
    void draw(const Navigation& navigation)
    {
        const std::array colors{Vector3{0.35, 0.35, 0.38}, Vector3{1, 0, 0},
                                Vector3{0, 1, 0}, Vector3{0, 0, 1}};
        glViewport(0, 0, 256, 256);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.bind();
        shader.setMatrix4("uModel", Matrix4::identity());
        shader.setMatrix4("uView", viewMatrix(navigation.camera));
        shader.setMatrix4("uProjection", navigation.projection.matrix(fov, 1, nearPlane, farPlane));
        for (std::size_t i = 0; i < lines.size(); ++i)
        {
            shader.setVector3("uColor", colors[i]);
            lines[i].draw();
            EXPECT_EQ(glGetError(), GL_NO_ERROR) << "batch " << i;
        }
    }
};

TEST_F(ViewerIntegrationGL, RealGridAxesAndNavigationRenderInBothModes)
{
    AidPass pass;
    ASSERT_FALSE(pass.grid.vertices().empty());
    for (std::size_t i = 0; i < pass.grid.vertices().size(); i += 2)
    {
        const auto& a = pass.grid.vertices()[i];
        const auto& b = pass.grid.vertices()[i + 1];
        EXPECT_TRUE(almostEqual(a.z(), 0.0) && almostEqual(b.z(), 0.0));
        EXPECT_FALSE(almostEqual(a.x(), 0.0) && almostEqual(b.x(), 0.0));
        EXPECT_FALSE(almostEqual(a.y(), 0.0) && almostEqual(b.y(), 0.0));
    }
    const std::array segments{pass.axes.xAxis(), pass.axes.yAxis(), pass.axes.zAxis()};
    const std::array directions{Vector3{1, 0, 0}, Vector3{0, 1, 0}, Vector3{0, 0, 1}};
    for (std::size_t i = 0; i < segments.size(); ++i)
    {
        EXPECT_TRUE(almostEqual(segments[i][0], Vector3{}));
        EXPECT_GT(dot(segments[i][1], directions[i]), 0);
        EXPECT_TRUE(almostEqual(segments[i][1].normalized(), directions[i]));
        EXPECT_EQ(pass.lines[i + 1].vertexCount(), segments[i].size());
    }
    EXPECT_EQ(pass.lines[0].vertexCount(), pass.grid.vertices().size());
    Navigation navigation;
    for (auto mode : modes)
    {
        navigation.projection.setMode(mode);
        pass.draw(navigation);
        navigation.drag(false);
        pass.draw(navigation);
        navigation.drag(true);
        pass.draw(navigation);
        navigation.wheel(2);
        pass.draw(navigation);
        expectView(navigation.camera);
        expectProjectedTarget(navigation);
    }
    navigation.projection.setMode(ProjectionMode::Perspective);
    pass.draw(navigation); // Same GPU batches survive the complete round trip.
}

TEST_F(ViewerIntegrationGL, PoleAndZoomLimitsRemainFiniteAndRenderable)
{
    AidPass pass;
    for (auto mode : modes)
        for (double pole : {-10000.0, 10000.0})
            for (double wheel : {-10000.0, 10000.0})
            {
                Navigation navigation;
                navigation.projection.setMode(mode);
                navigation.drag(false, 20, pole);
                navigation.wheel(wheel);
                EXPECT_GT(std::abs(navigation.camera.pitch()), 1.5);
                if (mode == ProjectionMode::Perspective)
                    EXPECT_DOUBLE_EQ(navigation.camera.distance(), wheel > 0
                        ? PanZoomNavigation::minimumDistance() : PanZoomNavigation::maximumDistance());
                else
                    EXPECT_DOUBLE_EQ(navigation.projection.visibleHeight(), wheel > 0
                        ? PanZoomNavigation::minimumVisibleHeight() : PanZoomNavigation::maximumVisibleHeight());
                expectView(navigation.camera);
                expectFinite(navigation.projection.matrix(fov, 1, nearPlane, farPlane));
                EXPECT_NO_THROW(pass.draw(navigation));
            }
}

TEST_F(ViewerIntegrationGL, WorkspaceRendersNavigationAndBlockedProjectionCommands)
{
    WorkspaceViewport viewport;
    EXPECT_EQ(viewport.projectionMode(), ProjectionMode::Perspective);
    const auto render = [&] {
        EXPECT_NO_THROW(viewport.render(layout, 256, 256));
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    };
    render();
    for (auto mode : modes)
    {
        auto input = pointer();
        input.blocked = true;
        input.projectionRequest = mode;
        input.wheelDelta = 2;
        viewport.updateNavigation(layout, input);
        // Explicit UI commands apply even while pointer navigation is blocked.
        EXPECT_EQ(viewport.projectionMode(), mode);
        render();
        for (bool pan : {false, true})
        {
            input = pointer();
            input.middlePressed = input.middleDown = true;
            input.shiftDown = pan;
            viewport.updateNavigation(layout, input);
            input.middlePressed = false;
            input.x += 20;
            input.y -= 10;
            viewport.updateNavigation(layout, input);
            render();
            input.middleDown = false;
            viewport.updateNavigation(layout, input);
        }
        input = pointer();
        input.wheelDelta = 2;
        viewport.updateNavigation(layout, input);
        render();
    }
}

// Query generated primitives rather than pixels: distinguish a real draw from
// an empty pass without depending on rasterization or exposing viewer internals.
struct PrimitiveQuery
{
    GLuint id{};
    PrimitiveQuery() { glGenQueries(1, &id); }
    ~PrimitiveQuery() { glDeleteQueries(1, &id); }
};

TEST_F(ViewerIntegrationGL, WorkspaceResizeHiDpiAndEmptyRegionsRenderOrNoOp)
{
    WorkspaceViewport viewport;
    PrimitiveQuery query;
    struct Surface { WorkspaceLayout layout; int width, height; bool draws; };
    const std::array surfaces{
        Surface{layout, 128, 128, true},
        Surface{layout, 256, 256, true},
        Surface{layout, 256, 128, true},
        Surface{{16, 16, 24, 96, 128, 128}, 128, 128, true},
        Surface{{16, 16, 96, 24, 128, 128}, 128, 128, true},
        Surface{{-16, 16, 64, 96, 128, 128}, 128, 128, true},
        Surface{{16, 16, 0, 96, 128, 128}, 128, 128, false},
        Surface{{16, 16, 96, 0, 128, 128}, 128, 128, false},
        Surface{layout, 0, 0, false},
        Surface{{200, 16, 96, 96, 128, 128}, 128, 128, false}};
    for (auto mode : modes)
    {
        viewport.setProjectionMode(mode);
        for (const auto& surface : surfaces)
        {
            SCOPED_TRACE(surface.draws);
            glViewport(3, 4, 100, 90);
            glScissor(5, 6, 80, 70);
            glBeginQuery(GL_PRIMITIVES_GENERATED, query.id);
            EXPECT_NO_THROW(viewport.render(surface.layout, surface.width, surface.height));
            glEndQuery(GL_PRIMITIVES_GENERATED);
            GLuint generated{};
            glGetQueryObjectuiv(query.id, GL_QUERY_RESULT, &generated);
            if (surface.draws) EXPECT_GT(generated, 0u);
            else EXPECT_EQ(generated, 0u);
            GLint viewportAfter[4]{}, scissorAfter[4]{};
            glGetIntegerv(GL_VIEWPORT, viewportAfter);
            glGetIntegerv(GL_SCISSOR_BOX, scissorAfter);
            const std::array expectedViewport{3, 4, 100, 90}, expectedScissor{5, 6, 80, 70};
            for (std::size_t i = 0; i < 4; ++i)
            {
                EXPECT_EQ(viewportAfter[i], expectedViewport[i]);
                EXPECT_EQ(scissorAfter[i], expectedScissor[i]);
            }
            EXPECT_EQ(viewport.projectionMode(), mode);
            EXPECT_EQ(glGetError(), GL_NO_ERROR);
        }
    }
}
}
