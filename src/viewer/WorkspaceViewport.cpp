#include "viewer/WorkspaceViewport.h"

#include "viewer/OrbitCamera.h"
#include "viewer/HoverState.h"
#include "viewer/SelectionState.h"
#include "viewer/HighlightColors.h"
#include "viewer/OrbitNavigation.h"
#include "viewer/PanZoomNavigation.h"
#include "viewer/ProjectionState.h"
#include "viewer/ReferenceAxes.h"
#include "viewer/ReferenceGrid.h"
#include "viewer/ViewProjection.h"
#include "viewer/PresentedPoints.h"
#include "viewer/PresentedSegments.h"
#include "viewer/PresentedLines.h"
#include "rendering/ShaderProgram.h"
#include "rendering/LineRenderer.h"
#include "rendering/PointRenderer.h"
#include "presentation/GeometryPresentation.h"
#include "presentation/WorldGeometry.h"
#include <limits>
#include <type_traits>

#include <glad/gl.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace microsw::viewer
{

math::Scalar ViewportRect::aspectRatio() const noexcept
{
    return width > 0 && height > 0 ? static_cast<math::Scalar>(width) / height : 0.0;
}

ViewportRect framebufferRect(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight)
{
    for (double value : {layout.x, layout.y, layout.width, layout.height,
                         layout.displayWidth, layout.displayHeight})
    {
        if (!std::isfinite(value))
            return {};
    }
    if (framebufferWidth <= 0 || framebufferHeight <= 0
        || layout.width <= 0 || layout.height <= 0
        || layout.displayWidth <= 0 || layout.displayHeight <= 0)
        return {};

    // Clip in logical units before scaling, avoiding integer overflow and UI bleed.
    const double left = std::clamp(layout.x, 0.0, layout.displayWidth);
    const double top = std::clamp(layout.y, 0.0, layout.displayHeight);
    const double right = std::clamp(layout.x + layout.width, 0.0, layout.displayWidth);
    const double bottom = std::clamp(layout.y + layout.height, 0.0, layout.displayHeight);
    const int x0 = static_cast<int>(std::ceil((left / layout.displayWidth) * framebufferWidth));
    const int y0 = static_cast<int>(std::ceil((top / layout.displayHeight) * framebufferHeight));
    const int x1 = static_cast<int>(std::floor((right / layout.displayWidth) * framebufferWidth));
    const int y1 = static_cast<int>(std::floor((bottom / layout.displayHeight) * framebufferHeight));
    if (x1 <= x0 || y1 <= y0)
        return {};
    return {x0, framebufferHeight - y1, x1 - x0, y1 - y0};
}

namespace
{

constexpr const char* vertexSource = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";
constexpr const char* fragmentSource = R"(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() { FragColor = vec4(uColor, 1.0); }
)";

// Local pass guard, including exception paths; not a general OpenGL state stack.
struct PassState
{
    GLint viewport[4]{}, scissor[4]{}, depthFunc{}, program{}, vao{};
    GLboolean scissorEnabled{}, depthEnabled{}, blendEnabled{}, depthMask{}, colorMask[4]{};
    GLfloat clearColor[4]{};
    GLdouble clearDepth{};

    PassState()
    {
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        depthEnabled = glIsEnabled(GL_DEPTH_TEST);
        blendEnabled = glIsEnabled(GL_BLEND);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
        glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clearDepth);
    }

    ~PassState()
    {
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        if (scissorEnabled) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (depthEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (blendEnabled) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        glDepthFunc(static_cast<GLenum>(depthFunc));
        glDepthMask(depthMask);
        glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClearDepth(clearDepth);
        glUseProgram(static_cast<GLuint>(program));
        glBindVertexArray(static_cast<GLuint>(vao));
    }
};

}

class WorkspaceViewport::Impl
{
public:
    explicit Impl(const presentation::GeometryPresentation* sourcePresentation)
        : presentation{sourcePresentation}, shader{vertexSource, fragmentSource}
    {
        const ReferenceAxes axes;
        const ReferenceGrid grid;
        gridRenderer.setVertices(grid.vertices());
        xAxis.setVertices(axes.xAxis());
        yAxis.setVertices(axes.yAxis());
        zAxis.setVertices(axes.zAxis());
    }

    OrbitCamera camera;
    HoverState hover;
    SelectionState selection;
    const presentation::GeometryPresentation* presentation;
    ProjectionState projection;
    OrbitNavigation navigation;
    PanZoomNavigation panZoomNavigation;
    const math::Scalar verticalFov = std::numbers::pi_v<math::Scalar> / 3.0;
    const math::Scalar nearPlane = 0.1;
    // Keep the origin aids within clipping range at the maximum zoom distance.
    const math::Scalar farPlane = PanZoomNavigation::maximumDistance() + 100.0;
    rendering::ShaderProgram shader;
    rendering::LineRenderer gridRenderer;
    rendering::LineRenderer xAxis;
    rendering::LineRenderer yAxis;
    rendering::LineRenderer zAxis;
    rendering::LineRenderer lines;
    rendering::LineRenderer segments;
    rendering::PointRenderer points;
};

WorkspaceViewport::WorkspaceViewport() : impl_{std::make_unique<Impl>(nullptr)} {}
WorkspaceViewport::WorkspaceViewport(const presentation::GeometryPresentation& presentation)
    : impl_{std::make_unique<Impl>(&presentation)} {}
WorkspaceViewport::~WorkspaceViewport() = default;

ProjectionMode WorkspaceViewport::projectionMode() const noexcept
{
    return impl_->projection.mode();
}

void WorkspaceViewport::setProjectionMode(ProjectionMode mode) noexcept
{
    impl_->projection.setMode(mode);
}

std::optional<PickHit> WorkspaceViewport::pick(
    const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
    math::Scalar mouseX, math::Scalar mouseY) const
{
    for (const auto value : {layout.x, layout.y, layout.width, layout.height,
                            layout.displayWidth, layout.displayHeight, mouseX, mouseY})
        if (!std::isfinite(value))
            throw std::invalid_argument{"Picking layout and mouse must be finite"};
    if (layout.width <= 0 || layout.height <= 0
        || layout.displayWidth <= 0 || layout.displayHeight <= 0)
        throw std::invalid_argument{"Picking layout dimensions must be positive"};
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
        throw std::invalid_argument{"Picking framebuffer dimensions must be positive"};
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0)
        return std::nullopt;
    const auto logicalPerPixelX = layout.displayWidth / framebufferWidth;
    const auto logicalPerPixelY = layout.displayHeight / framebufferHeight;

    PickingContext context;
    context.camera = impl_->camera;
    context.projection = impl_->projection;
    context.width = rect.width * logicalPerPixelX;
    context.height = rect.height * logicalPerPixelY;
    context.mouseX = mouseX - rect.x * logicalPerPixelX;
    context.mouseY = mouseY - (framebufferHeight - rect.y - rect.height) * logicalPerPixelY;
    context.projectionAspectRatio = rect.aspectRatio();
    if (!std::isfinite(context.mouseX) || !std::isfinite(context.mouseY))
        throw std::overflow_error{"Workspace-local mouse is not representable"};
    context.verticalFov = impl_->verticalFov;
    context.nearPlane = impl_->nearPlane;
    context.farPlane = impl_->farPlane;
    context.validate();
    if (!impl_->presentation || mouseX < 0 || mouseX >= layout.displayWidth
        || mouseY < 0 || mouseY >= layout.displayHeight)
        return std::nullopt;
    return pickGeometry(*impl_->presentation, context);
}

void WorkspaceViewport::updateNavigation(const WorkspaceLayout& layout, const WorkspaceInput& input)
{
    if (input.projectionRequest)
        setProjectionMode(*input.projectionRequest);
    impl_->navigation.handle(layout, input, impl_->camera);
    impl_->panZoomNavigation.handle(layout, input, impl_->camera, impl_->projection);
}

std::optional<presentation::VisualEntityId> WorkspaceViewport::hoveredEntity() const noexcept
{
    return impl_->hover.hovered();
}

void WorkspaceViewport::updateHover(const WorkspaceLayout& layout, const WorkspaceInput& input,
    int framebufferWidth, int framebufferHeight)
{
    // Clear first so unavailable interaction or a failed query cannot leave a
    // stale identity. PickHit and its distance never survive this update.
    impl_->hover.clear();
    if (!input.focused || !input.pointerValid || !input.workspaceHovered || input.blocked
        || input.middleDown || input.middlePressed || !contains(layout, input.x, input.y))
        return;
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0)
        return; // Normal minimize/resize frames must not call the strict picker.
    if (const auto hit = pick(layout, framebufferWidth, framebufferHeight, input.x, input.y))
        impl_->hover.update(hit->id);
}

std::optional<presentation::VisualEntityId> WorkspaceViewport::selectedEntity() const noexcept
{
    return impl_->selection.selected();
}

void WorkspaceViewport::updateSelection(const WorkspaceLayout& layout, const WorkspaceInput& input,
    int framebufferWidth, int framebufferHeight)
{
    if (!input.leftPressed || !input.focused || !input.pointerValid
        || !input.workspaceHovered || input.blocked || input.middleDown || input.middlePressed
        || !contains(layout, input.x, input.y))
        return;
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0)
        return;

    // Distinguish an empty hit from input outside the actual raster viewport,
    // including the fractional strips excluded by inward HiDPI rounding.
    const auto scaleX = layout.displayWidth / framebufferWidth;
    const auto scaleY = layout.displayHeight / framebufferHeight;
    const auto x = input.x - rect.x * scaleX;
    const auto y = input.y - (framebufferHeight - rect.y - rect.height) * scaleY;
    if (x < 0 || x >= rect.width * scaleX || y < 0 || y >= rect.height * scaleY)
        return;
    if (const auto hit = pick(layout, framebufferWidth, framebufferHeight, input.x, input.y))
        impl_->selection.select(hit->id);
    else
        impl_->selection.clear();
}

void WorkspaceViewport::validateTransform(presentation::VisualEntityId id, const math::Transform3& transform) const
{
    const auto* entity = impl_->presentation ? impl_->presentation->find(id) : nullptr;
    if (!entity) throw std::invalid_argument{"Unknown visual entity"};
    auto candidate = *entity;
    candidate.setTransform(transform);
    const auto world = presentation::worldGeometry(candidate);
    const auto check = [](const geometry::Point3& point)
    {
        // Reserve headroom for view-derived line endpoints and GPU arithmetic.
        for (const auto value : {point.x(), point.y(), point.z()})
            if (std::abs(value) > std::numeric_limits<float>::max() / 4.0)
                throw std::invalid_argument{"Position exceeds the viewer's numeric range"};
    };
    std::visit([&](const auto& value)
    {
        using Geometry = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Geometry, geometry::Point3>) check(value);
        else if constexpr (std::is_same_v<Geometry, geometry::Segment3>) { check(value.a()); check(value.b()); }
        else check(value.origin());
    }, world);
}

void WorkspaceViewport::render(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight)
{
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0)
        return;

    const PassState saved;
    glViewport(rect.x, rect.y, rect.width, rect.height);
    glScissor(rect.x, rect.y, rect.width, rect.height);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_BLEND);
    glClearColor(0.12F, 0.12F, 0.14F, 1.0F);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    impl_->shader.bind();
    impl_->shader.setMatrix4("uModel", math::Matrix4::identity());
    impl_->shader.setMatrix4("uView", viewMatrix(impl_->camera));
    impl_->shader.setMatrix4("uProjection",
        impl_->projection.matrix(impl_->verticalFov, rect.aspectRatio(), impl_->nearPlane, impl_->farPlane));
    const math::Scalar visibleHeight = impl_->projection.mode() == ProjectionMode::Perspective
        ? 2.0 * impl_->camera.distance() * std::tan(impl_->verticalFov / 2.0)
        : impl_->projection.visibleHeight();
    const LinePresentationContext lineContext{impl_->camera.target(),
        visibleHeight * std::max<math::Scalar>(1.0, rect.aspectRatio())};

    // Reuse three generic GPU buffers for successive state batches. IDs stay on CPU.
    const auto upload = [&](VisualState state)
    {
        if (!impl_->presentation) return;
        const VisualStateFilter filter{state, impl_->hover.hovered(), impl_->selection.selected()};
        impl_->lines.setVertices(presentedLineVertices(*impl_->presentation, lineContext, filter));
        impl_->segments.setVertices(presentedSegmentVertices(*impl_->presentation, filter));
        impl_->points.setVertices(presentedPointVertices(*impl_->presentation, filter));
    };
    const auto drawLines = [&](VisualState state)
    {
        impl_->shader.setVector3("uColor", highlightColor(state, normalLineColor));
        impl_->lines.draw();
    };
    const auto drawSegmentsAndPoints = [&](VisualState state)
    {
        impl_->shader.setVector3("uColor", highlightColor(state, normalSegmentColor));
        impl_->segments.draw();
        impl_->shader.setVector3("uColor", highlightColor(state, normalPointColor));
        impl_->points.draw();
    };

    // Preserve B4.8's normal pass order and depth policy when no state is active.
    impl_->shader.setVector3("uColor", {0.35, 0.35, 0.38});
    impl_->gridRenderer.draw();
    upload(VisualState::Normal);
    drawLines(VisualState::Normal);
    impl_->shader.setVector3("uColor", {1.0, 0.0, 0.0});
    impl_->xAxis.draw();
    impl_->shader.setVector3("uColor", {0.0, 1.0, 0.0});
    impl_->yAxis.draw();
    impl_->shader.setVector3("uColor", {0.0, 0.0, 1.0});
    impl_->zAxis.draw();
    drawSegmentsAndPoints(VisualState::Normal);

    // Equal-depth fragments follow state precedence; nearer geometry still occludes.
    // No depth offset, depth clear or duplicate entity is needed for highlighting.
    glDepthFunc(GL_LEQUAL);
    for (const auto state : {VisualState::Hovered, VisualState::Selected})
    {
        upload(state);
        drawLines(state);
        drawSegmentsAndPoints(state);
    }
}

}
