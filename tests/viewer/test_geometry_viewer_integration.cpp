#include "app/demo/GeometryDemoScene.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "rendering/ShaderProgram.h"
#include "viewer/WorkspaceViewport.h"
#include "viewer/PresentedPoints.h"
#include "viewer/PresentedSegments.h"
#include "viewer/PresentedLines.h"
#include "viewer/HighlightColors.h"
#include "viewer/ReferenceGrid.h"
#include "viewer/ReferenceAxes.h"
#include "viewer/ViewProjection.h"
#include "viewer/OrbitNavigation.h"
#include "viewer/PanZoomNavigation.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>
#include <array>
#include <cmath>
#include <memory>
#include <numbers>
#include <tuple>
#include <span>
#include <type_traits>

namespace
{
using namespace microsw;
using namespace microsw::viewer;
using presentation::GeometryPresentation;
using presentation::VisualEntityId;
using geometry::Point3;
using geometry::Segment3;
using geometry::Line3;
using math::Vector3;

Point3 point(const Vector3& v) { return {v.x(), v.y(), v.z()}; }

// Read-only observation of the real draw path. Forward every draw to the driver
// and restore GLAD on scope exit; no replacement renderer or pixel assertions.
struct DrawObservation
{
    GLenum primitive;
    GLint depth;
    GLboolean depthEnabled, depthMask;
    std::array<GLfloat, 3> color{};
    std::vector<GLfloat> vertices;
};
class DrawTrace
{
public:
    DrawTrace() : original_{glad_glDrawArrays} { active_ = this; glad_glDrawArrays = observe; }
    ~DrawTrace() { glad_glDrawArrays = original_; active_ = nullptr; }
    DrawTrace(const DrawTrace&) = delete;
    DrawTrace& operator=(const DrawTrace&) = delete;
    std::vector<DrawObservation> draws;
private:
    static void GLAD_API_PTR observe(GLenum mode, GLint first, GLsizei count)
    {
        DrawObservation draw{};
        draw.primitive = mode;
        glGetIntegerv(GL_DEPTH_FUNC, &draw.depth);
        draw.depthEnabled = glIsEnabled(GL_DEPTH_TEST);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &draw.depthMask);
        GLint program{}, buffer{}, previousBuffer{};
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetUniformfv(program, glGetUniformLocation(program, "uColor"), draw.color.data());
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        draw.vertices.resize(static_cast<std::size_t>(count) * 3);
        glGetBufferSubData(GL_ARRAY_BUFFER, first * 3 * sizeof(GLfloat),
            count * 3 * sizeof(GLfloat), draw.vertices.data());
        glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
        active_->draws.push_back(std::move(draw));
        active_->original_(mode, first, count);
    }
    inline static DrawTrace* active_{};
    PFNGLDRAWARRAYSPROC original_;
};

// Snapshot all stored payload components, IDs, alternatives and insertion order.
// Exact equality is intentional here: observation must not rewrite stored values.
auto snapshot(const GeometryPresentation& presentation)
{
    std::vector<std::tuple<std::uint64_t, std::size_t, std::vector<double>>> result;
    for (const auto& entity : presentation.entities())
    {
        std::vector<double> values;
        const auto append = [&](const auto& v)
        { values.insert(values.end(), {v.x(), v.y(), v.z()}); };
        std::visit([&](const auto& value)
        {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Point3>) append(value);
            else if constexpr (std::is_same_v<T, Segment3>) { append(value.a()); append(value.b()); }
            else { append(value.origin()); append(value.direction()); }
        }, entity.geometry());
        result.emplace_back(entity.id().value(), entity.geometry().index(), std::move(values));
    }
    return result;
}

class GeometryViewerIntegration : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<ApplicationWindow>(640, 480, "Geometry integration tests");
        glfwHideWindow(static_cast<GLFWwindow*>(window->nativeHandle()));
        context = std::make_unique<OpenGLContext>(*window);
    }
    void TearDown() override
    {
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
        context.reset();
        window.reset();
    }
    // Public navigation components supply a reference view snapshot; Workspace
    // orchestration must consume that same input before picking and drawing.
    void frame(WorkspaceViewport& workspace, const WorkspaceInput& input)
    {
        if (input.projectionRequest) projection.setMode(*input.projectionRequest);
        orbit.handle(layout, input, camera);
        panZoom.handle(layout, input, camera, projection);
        workspace.updateNavigation(layout, input);
        workspace.updateHover(layout, input, width, height);
        workspace.updateSelection(layout, input, width, height);
        draw(workspace);
    }
    void draw(WorkspaceViewport& workspace)
    {
        DrawTrace trace;
        workspace.render(layout, width, height);
        draws = std::move(trace.draws);
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
    LinePresentationContext lineContext() const
    {
        const auto rect = framebufferRect(layout, width, height);
        const auto visibleHeight = projection.mode() == ProjectionMode::Perspective
            ? 2 * camera.distance() * std::tan(std::numbers::pi / 6) : projection.visibleHeight();
        return {camera.target(), visibleHeight * std::max(1.0, rect.aspectRatio())};
    }
    WorkspaceInput at(const Point3& world, bool press = false) const
    {
        const auto rect = framebufferRect(layout, width, height);
        const auto matrix = projection.matrix(std::numbers::pi / 3, rect.aspectRatio(), 0.1, 1100)
            * viewMatrix(camera);
        const std::array<double, 4> position{world.x(), world.y(), world.z(), 1};
        std::array<double, 4> clip{};
        for (std::size_t row = 0; row < 4; ++row)
            for (std::size_t column = 0; column < 4; ++column)
                clip[row] += matrix(row, column) * position[column];
        const auto scaleX = layout.displayWidth / width;
        const auto scaleY = layout.displayHeight / height;
        WorkspaceInput input;
        input.x = rect.x * scaleX + (clip[0] / clip[3] + 1) * rect.width * scaleX / 2;
        input.y = (height - rect.y - rect.height) * scaleY
            + (1 - clip[1] / clip[3]) * rect.height * scaleY / 2;
        input.focused = input.pointerValid = input.workspaceHovered = true;
        input.leftPressed = press;
        return input;
    }
    void expectDraw(std::size_t& index, GLenum primitive, std::span<const Vector3> vertices,
        const Vector3& color, GLint depth)
    {
        if (vertices.empty()) return;
        ASSERT_LT(index, draws.size());
        const auto& actual = draws[index++];
        EXPECT_EQ(actual.primitive, primitive);
        EXPECT_EQ(actual.depth, depth);
        EXPECT_EQ(actual.depthEnabled, GL_TRUE);
        EXPECT_EQ(actual.depthMask, GL_TRUE);
        EXPECT_FLOAT_EQ(actual.color[0], static_cast<float>(color.x()));
        EXPECT_FLOAT_EQ(actual.color[1], static_cast<float>(color.y()));
        EXPECT_FLOAT_EQ(actual.color[2], static_cast<float>(color.z()));
        ASSERT_EQ(actual.vertices.size(), vertices.size() * 3);
        for (std::size_t i = 0; i < vertices.size(); ++i)
        {
            EXPECT_FLOAT_EQ(actual.vertices[3 * i], static_cast<float>(vertices[i].x()));
            EXPECT_FLOAT_EQ(actual.vertices[3 * i + 1], static_cast<float>(vertices[i].y()));
            EXPECT_FLOAT_EQ(actual.vertices[3 * i + 2], static_cast<float>(vertices[i].z()));
        }
    }
    void expectBatches(const GeometryPresentation& presentation, const WorkspaceViewport& workspace)
    {
        std::size_t index = 0, pointCount = 0, segmentCount = 0, lineCount = 0;
        expectDraw(index, GL_LINES, ReferenceGrid{}.vertices(), {0.35, 0.35, 0.38}, GL_LESS);
        for (auto state : {VisualState::Normal, VisualState::Hovered, VisualState::Selected})
        {
            const VisualStateFilter filter{state, workspace.hoveredEntity(), workspace.selectedEntity()};
            const auto lines = presentedLineVertices(presentation, lineContext(), filter);
            const auto segments = presentedSegmentVertices(presentation, filter);
            const auto points = presentedPointVertices(presentation, filter);
            const auto depth = state == VisualState::Normal ? GL_LESS : GL_LEQUAL;
            expectDraw(index, GL_LINES, lines, highlightColor(state, normalLineColor), depth);
            if (state == VisualState::Normal)
            {
                const ReferenceAxes axes;
                expectDraw(index, GL_LINES, axes.xAxis(), {1, 0, 0}, GL_LESS);
                expectDraw(index, GL_LINES, axes.yAxis(), {0, 1, 0}, GL_LESS);
                expectDraw(index, GL_LINES, axes.zAxis(), {0, 0, 1}, GL_LESS);
            }
            expectDraw(index, GL_LINES, segments, highlightColor(state, normalSegmentColor), depth);
            expectDraw(index, GL_POINTS, points, highlightColor(state, normalPointColor), depth);
            pointCount += points.size(); segmentCount += segments.size(); lineCount += lines.size();
        }
        EXPECT_EQ(index, draws.size());
        EXPECT_EQ(pointCount, presentedPointVertices(presentation).size());
        EXPECT_EQ(segmentCount, presentedSegmentVertices(presentation).size());
        EXPECT_EQ(lineCount, presentedLineVertices(presentation, lineContext()).size());
    }
    void demoSlice(std::size_t entityIndex, const Point3& sample)
    {
        const auto demo = demo::createGeometryDemoScene();
        const auto before = snapshot(demo);
        const auto id = demo.entities()[entityIndex].id();
        WorkspaceViewport workspace{demo};
        EXPECT_EQ(presentedPointVertices(demo).size(), 3U);
        EXPECT_EQ(presentedSegmentVertices(demo).size(), 6U);
        EXPECT_EQ(presentedLineVertices(demo, lineContext()).size(), 4U);
        auto input = at(sample);
        draw(workspace);
        expectBatches(demo, workspace);
        const auto hit = workspace.pick(layout, width, height, input.x, input.y);
        ASSERT_TRUE(hit);
        ASSERT_EQ(hit->id, id);
        frame(workspace, input);
        EXPECT_EQ(workspace.hoveredEntity(), id);
        EXPECT_FALSE(workspace.selectedEntity());
        expectBatches(demo, workspace);
        input.leftPressed = true;
        frame(workspace, input);
        EXPECT_EQ(workspace.selectedEntity(), id);
        EXPECT_EQ(visualStateFor(id, workspace.hoveredEntity(), workspace.selectedEntity()), VisualState::Selected);
        expectBatches(demo, workspace);
        EXPECT_EQ(snapshot(demo), before);
    }
    WorkspaceLayout layout{16, 16, 608, 448, 640, 480};
    int width = 640, height = 480;
    OrbitCamera camera;
    ProjectionState projection;
    OrbitNavigation orbit;
    PanZoomNavigation panZoom;
    std::vector<DrawObservation> draws;
    std::unique_ptr<ApplicationWindow> window;
    std::unique_ptr<OpenGLContext> context;
};

TEST_F(GeometryViewerIntegration, DemoPointIdentityReachesActualSelectedDraw)
{ demoSlice(1, {2, 2, 1}); }
TEST_F(GeometryViewerIntegration, DemoSegmentMidpointReachesActualSelectedDraw)
{ demoSlice(3, {0, -2, 0.5}); }
TEST_F(GeometryViewerIntegration, DemoLineIdentityReachesViewDerivedSelectedDraw)
{ demoSlice(6, {-2, 3, 1}); }

TEST_F(GeometryViewerIntegration, DemoHoverSelectionAndEmptyClickPartitionEveryPrimitiveExclusively)
{
    const auto demo = demo::createGeometryDemoScene();
    const auto before = snapshot(demo);
    WorkspaceViewport workspace{demo};
    const std::array<Point3, 3> samples{Point3{2, 2, 1}, Point3{0, -2, 0.5}, Point3{-2, 3, 1}};
    const std::array<std::size_t, 3> indices{1, 3, 6};
    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        frame(workspace, at(samples[i], true));
        const auto selected = demo.entities()[indices[i]].id();
        ASSERT_EQ(workspace.selectedEntity(), selected);
        frame(workspace, at(samples[(i + 1) % 3]));
        const auto hovered = demo.entities()[indices[(i + 1) % 3]].id();
        ASSERT_EQ(workspace.hoveredEntity(), hovered);
        for (const auto& entity : demo.entities())
            EXPECT_EQ(visualStateFor(entity.id(), workspace.hoveredEntity(), workspace.selectedEntity()),
                entity.id() == selected ? VisualState::Selected
                    : entity.id() == hovered ? VisualState::Hovered : VisualState::Normal);
        expectBatches(demo, workspace);
    }
    auto empty = at(Point3{});
    empty.x = 22; empty.y = 22; empty.leftPressed = true;
    ASSERT_FALSE(workspace.pick(layout, width, height, empty.x, empty.y));
    frame(workspace, empty);
    EXPECT_FALSE(workspace.selectedEntity());
    EXPECT_FALSE(workspace.hoveredEntity());
    expectBatches(demo, workspace);
    frame(workspace, at(samples[0]));
    EXPECT_EQ(workspace.hoveredEntity(), demo.entities()[1].id());
    EXPECT_FALSE(workspace.selectedEntity());
    expectBatches(demo, workspace);
    EXPECT_EQ(snapshot(demo), before);
}

TEST_F(GeometryViewerIntegration, NavigationUpdatesLineDrawAndReleasesHoverWithoutLosingIdentity)
{
    const auto demo = demo::createGeometryDemoScene();
    const auto before = snapshot(demo);
    WorkspaceViewport workspace{demo};
    frame(workspace, at({-2, 3, 1}, true));
    const auto selected = demo.entities()[6].id();
    ASSERT_EQ(workspace.selectedEntity(), selected);
    for (bool pan : {false, true})
    {
        const auto oldLines = presentedLineVertices(demo, lineContext());
        auto drag = at(Point3{});
        drag.middlePressed = drag.middleDown = true; drag.shiftDown = pan;
        frame(workspace, drag);
        drag.middlePressed = false; drag.x += 35; drag.y += 15;
        frame(workspace, drag);
        EXPECT_FALSE(workspace.hoveredEntity());
        EXPECT_EQ(workspace.selectedEntity(), selected);
        expectBatches(demo, workspace);
        if (pan) EXPECT_FALSE(math::almostEqual(oldLines[0], presentedLineVertices(demo, lineContext())[0]));
        // Release onto a known projected demo point in the resulting view.
        const auto release = at({2, 2, 1});
        frame(workspace, release);
        const auto hit = workspace.pick(layout, width, height, release.x, release.y);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, demo.entities()[1].id());
        EXPECT_EQ(workspace.hoveredEntity(), hit->id);
        EXPECT_EQ(workspace.selectedEntity(), selected);
        expectBatches(demo, workspace);
    }
    const auto oldLines = presentedLineVertices(demo, lineContext());
    auto zoom = at(Point3{}); zoom.wheelDelta = 2;
    frame(workspace, zoom);
    EXPECT_FALSE(math::almostEqual(oldLines[0], presentedLineVertices(demo, lineContext())[0]));
    EXPECT_EQ(workspace.selectedEntity(), selected);
    expectBatches(demo, workspace);
    EXPECT_EQ(snapshot(demo), before);
}

TEST_F(GeometryViewerIntegration, ProjectionSwitchRecalculatesLineAndPickingWithPersistentHighlight)
{
    const auto demo = demo::createGeometryDemoScene();
    const auto before = snapshot(demo);
    WorkspaceViewport workspace{demo};
    frame(workspace, at({-2, 3, 1}, true));
    const auto id = demo.entities()[6].id();
    for (auto mode : {ProjectionMode::Orthographic, ProjectionMode::Perspective})
    {
        const auto oldLines = presentedLineVertices(demo, lineContext());
        auto request = at({2, 2, 1}); request.projectionRequest = mode;
        frame(workspace, request);
        const auto hit = workspace.pick(layout, width, height, request.x, request.y);
        EXPECT_EQ(workspace.hoveredEntity(), hit ? std::optional{hit->id} : std::nullopt);
        EXPECT_FALSE(math::almostEqual(oldLines[0], presentedLineVertices(demo, lineContext())[0]));
        frame(workspace, at({-2, 3, 1}));
        EXPECT_EQ(workspace.hoveredEntity(), id);
        EXPECT_EQ(workspace.selectedEntity(), id);
        expectBatches(demo, workspace);
    }
    EXPECT_EQ(snapshot(demo), before);
}

TEST_F(GeometryViewerIntegration, ResizeHiDpiAndClippedViewportKeepTheWholeSliceCoherent)
{
    GeometryPresentation presentation;
    const auto id = presentation.add(point(camera.right()));
    WorkspaceViewport workspace{presentation};
    for (const auto& size : std::array{std::array{640, 480}, std::array{1280, 960}, std::array{800, 720}})
        for (const auto& resized : {WorkspaceLayout{16.25, 16.25, 600.5, 440.5, 640, 480},
                                    WorkspaceLayout{-30.25, -20.25, 600.5, 440.5, 640, 480}})
        {
            width = size[0]; height = size[1]; layout = resized;
            auto input = at(point(camera.right()), true);
            const auto hit = workspace.pick(layout, width, height, input.x, input.y);
            ASSERT_TRUE(hit);
            EXPECT_EQ(hit->id, id);
            frame(workspace, input);
            EXPECT_EQ(workspace.hoveredEntity(), id);
            EXPECT_EQ(workspace.selectedEntity(), id);
            expectBatches(presentation, workspace);
        }
}

TEST_F(GeometryViewerIntegration, MinimizeClearsOnlyHoverAndRestoreRecalculatesIt)
{
    const auto demo = demo::createGeometryDemoScene();
    WorkspaceViewport workspace{demo};
    const auto input = at({2, 2, 1}, true);
    frame(workspace, input);
    const auto id = demo.entities()[1].id();
    ASSERT_EQ(workspace.selectedEntity(), id);
    const auto valid = layout;
    for (int kind = 0; kind < 3; ++kind)
    {
        if (kind == 0) layout = {};
        if (kind == 1) layout.width = 0;
        if (kind == 2) width = height = 0;
        EXPECT_NO_THROW(frame(workspace, input));
        EXPECT_TRUE(draws.empty());
        EXPECT_FALSE(workspace.hoveredEntity());
        EXPECT_EQ(workspace.selectedEntity(), id);
        layout = valid; width = 640; height = 480;
        frame(workspace, at({2, 2, 1}));
        EXPECT_EQ(workspace.hoveredEntity(), id);
        EXPECT_EQ(workspace.selectedEntity(), id);
        expectBatches(demo, workspace);
    }
}

TEST_F(GeometryViewerIntegration, UiModalBlockingPreservesSelectionAndUnblockRestoresHover)
{
    const auto demo = demo::createGeometryDemoScene();
    const auto before = snapshot(demo);
    WorkspaceViewport workspace{demo};
    frame(workspace, at({2, 2, 1}, true));
    const auto selected = demo.entities()[1].id();
    for (int gate = 0; gate < 4; ++gate)
    {
        auto input = at({0, -2, 0.5});
        frame(workspace, input);
        ASSERT_EQ(workspace.hoveredEntity(), demo.entities()[3].id());
        auto blocked = input; blocked.leftPressed = true;
        if (gate == 0) blocked.blocked = true;
        if (gate == 1) blocked.focused = false;
        if (gate == 2) blocked.pointerValid = false;
        if (gate == 3) blocked.workspaceHovered = false;
        frame(workspace, blocked);
        EXPECT_FALSE(workspace.hoveredEntity());
        EXPECT_EQ(workspace.selectedEntity(), selected);
        expectBatches(demo, workspace);
        frame(workspace, input);
        EXPECT_EQ(workspace.hoveredEntity(), demo.entities()[3].id());
        EXPECT_EQ(workspace.selectedEntity(), selected);
    }
    EXPECT_EQ(snapshot(demo), before);
}

TEST_F(GeometryViewerIntegration, LogicalPixelToleranceSurvivesZoomProjectionAndFramebufferScale)
{
    GeometryPresentation presentation;
    const auto id = presentation.add(Point3{});
    WorkspaceViewport workspace{presentation};
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
        for (int framebufferScale : {1, 2})
            for (double wheel : {-3.0, 2.0, 1.0})
            {
                width = 640 * framebufferScale; height = 480 * framebufferScale;
                auto zoom = at(Point3{}); zoom.projectionRequest = mode; zoom.wheelDelta = wheel;
                frame(workspace, zoom);
                for (double offset : {4.0, 8.0})
                {
                    auto input = at(Point3{}, true); input.x += offset;
                    const auto hit = workspace.pick(layout, width, height, input.x, input.y);
                    EXPECT_EQ(hit.has_value(), offset == 4);
                    frame(workspace, input);
                    const std::optional<VisualEntityId> expected = offset == 4 ? std::optional{id} : std::nullopt;
                    EXPECT_EQ(workspace.hoveredEntity(), expected);
                    EXPECT_EQ(workspace.selectedEntity(), expected);
                    expectBatches(presentation, workspace);
                }
            }
}

TEST_F(GeometryViewerIntegration, DeterministicWinnerFlowsThroughHoverSelectionAndExclusiveDraw)
{
    GeometryPresentation presentation;
    const auto first = presentation.add(Point3{});
    (void)presentation.add(Point3{});
    (void)presentation.add(Segment3{point(-camera.right()), point(camera.right())});
    (void)presentation.add(Line3{Point3{}, camera.right()});
    WorkspaceViewport workspace{presentation};
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        const auto input = at(Point3{}, true);
        const auto hit = workspace.pick(layout, width, height, input.x, input.y);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, first);
        frame(workspace, input);
        EXPECT_EQ(workspace.hoveredEntity(), hit->id);
        EXPECT_EQ(workspace.selectedEntity(), hit->id);
        expectBatches(presentation, workspace);
    }
}

TEST_F(GeometryViewerIntegration, DefaultAndEmptyWorkspacesKeepNavigationAndRenderingSafe)
{
    const GeometryPresentation empty;
    WorkspaceViewport defaultWorkspace, emptyWorkspace{empty};
    for (auto* workspace : {&defaultWorkspace, &emptyWorkspace})
        for (auto mode : {ProjectionMode::Orthographic, ProjectionMode::Perspective})
        {
            auto input = at(Point3{}, true); input.projectionRequest = mode; input.wheelDelta = 1;
            frame(*workspace, input);
            EXPECT_EQ(workspace->projectionMode(), mode);
            EXPECT_FALSE(workspace->pick(layout, width, height, input.x, input.y));
            EXPECT_FALSE(workspace->hoveredEntity());
            EXPECT_FALSE(workspace->selectedEntity());
            expectBatches(empty, *workspace);
        }
}

TEST_F(GeometryViewerIntegration, FiniteLineEndpointControlsPickingHoverSelectionAndActualDraw)
{
    GeometryPresentation presentation;
    const auto origin = point(camera.forward() * 490);
    const auto id = presentation.add(Line3{origin, camera.right()});
    const auto before = snapshot(presentation);
    WorkspaceViewport workspace{presentation};
    for (double wheel : {0.0, 2.0})
    {
        auto zoom = at(Point3{}); zoom.wheelDelta = wheel;
        frame(workspace, zoom);
        const auto vertices = presentedLineVertices(presentation, lineContext());
        ASSERT_EQ(vertices.size(), 2U);
        auto endpoint = at(point(vertices[1]), true);
        endpoint.x += 4;
        frame(workspace, endpoint);
        EXPECT_EQ(workspace.hoveredEntity(), id);
        EXPECT_EQ(workspace.selectedEntity(), id);
        expectBatches(presentation, workspace);
        endpoint.x += 4; // Still on the infinite support, beyond finite extent + tolerance.
        ASSERT_TRUE(contains(layout, endpoint.x, endpoint.y));
        EXPECT_FALSE(workspace.pick(layout, width, height, endpoint.x, endpoint.y));
        frame(workspace, endpoint);
        EXPECT_FALSE(workspace.hoveredEntity());
        EXPECT_FALSE(workspace.selectedEntity());
        expectBatches(presentation, workspace);
    }
    EXPECT_EQ(snapshot(presentation), before);
}
TEST_F(GeometryViewerIntegration, HighlightPassRestoresAllAssumedExternalGlState)
{
    const auto demo = demo::createGeometryDemoScene();
    WorkspaceViewport workspace{demo};
    frame(workspace, at({2, 2, 1}, true));
    frame(workspace, at({0, -2, 0.5}));
    rendering::ShaderProgram external{
        "#version 330 core\nlayout(location=0) in vec3 p; void main(){gl_Position=vec4(p,1);}",
        "#version 330 core\nout vec4 c; void main(){c=vec4(1);}"};
    external.bind();
    GLint program{}; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    GLuint vao{}; glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    glViewport(1, 2, 30, 40); glScissor(5, 6, 70, 80);
    glEnable(GL_SCISSOR_TEST); glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND);
    glDepthFunc(GL_GREATER); glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
    glClearColor(0.25F, 0.5F, 0.75F, 1); glClearDepth(0.5);
    draw(workspace);
    expectBatches(demo, workspace); // Also observes GL_LESS then GL_LEQUAL during actual draws.
    GLint actual[4]{};
    glGetIntegerv(GL_VIEWPORT, actual);
    EXPECT_EQ((std::array{actual[0], actual[1], actual[2], actual[3]}), (std::array{1, 2, 30, 40}));
    glGetIntegerv(GL_SCISSOR_BOX, actual);
    EXPECT_EQ((std::array{actual[0], actual[1], actual[2], actual[3]}), (std::array{5, 6, 70, 80}));
    EXPECT_EQ(glIsEnabled(GL_SCISSOR_TEST), GL_TRUE);
    EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_FALSE);
    EXPECT_EQ(glIsEnabled(GL_BLEND), GL_TRUE);
    glGetIntegerv(GL_DEPTH_FUNC, actual); EXPECT_EQ(actual[0], GL_GREATER);
    glGetIntegerv(GL_CURRENT_PROGRAM, actual); EXPECT_EQ(actual[0], program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, actual); EXPECT_EQ(actual[0], static_cast<GLint>(vao));
    GLboolean mask[4]{};
    glGetBooleanv(GL_DEPTH_WRITEMASK, mask); EXPECT_EQ(mask[0], GL_FALSE);
    glGetBooleanv(GL_COLOR_WRITEMASK, mask);
    EXPECT_EQ((std::array{mask[0], mask[1], mask[2], mask[3]}),
        (std::array<GLboolean, 4>{GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE}));
    GLfloat color[4]{}; glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    EXPECT_EQ((std::array{color[0], color[1], color[2], color[3]}), (std::array{0.25F, 0.5F, 0.75F, 1.0F}));
    GLdouble depth{}; glGetDoublev(GL_DEPTH_CLEAR_VALUE, &depth); EXPECT_DOUBLE_EQ(depth, 0.5);
    glBindVertexArray(0); glDeleteVertexArrays(1, &vao);
}
}
