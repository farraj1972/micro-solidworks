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
#include "viewer/PickingRay.h"
#include "viewer/SolidPicker.h"
#include "rendering/ShaderProgram.h"
#include "rendering/LineRenderer.h"
#include "rendering/PointRenderer.h"
#include "presentation/GeometryPresentation.h"
#include "presentation/FeaturePresentation.h"
#include "presentation/SolidDisplay.h"
#include "rendering/TriangleRenderer.h"
#include "presentation/WorldGeometry.h"
#include <algorithm>
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
constexpr const char* litVertexSource = R"(#version 330 core
layout(location=0) in vec3 aPosition; layout(location=1) in vec3 aNormal;
uniform mat4 uModel; uniform mat4 uView; uniform mat4 uProjection;
out vec3 vNormal;
void main(){vNormal=mat3(uModel)*aNormal;gl_Position=uProjection*uView*uModel*vec4(aPosition,1.0);})";
constexpr const char* litFragmentSource = R"(#version 330 core
in vec3 vNormal; out vec4 FragColor; uniform vec3 uColor;
void main(){vec3 n=normalize(vNormal);float d=max(dot(n,normalize(vec3(0.35,-0.45,0.82))),0.0);float l=0.32+0.68*d;FragColor=vec4(uColor*l,1.0);})";

// Local pass guard, including exception paths; not a general OpenGL state stack.
struct PassState
{
    GLint viewport[4]{}, scissor[4]{}, depthFunc{}, program{}, vao{};
    GLboolean scissorEnabled{}, depthEnabled{}, blendEnabled{}, polygonOffsetEnabled{}, depthMask{}, colorMask[4]{};
    GLfloat clearColor[4]{}, lineWidth{}, polygonOffsetFactor{}, polygonOffsetUnits{};
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
        polygonOffsetEnabled = glIsEnabled(GL_POLYGON_OFFSET_FILL);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
        glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        glGetFloatv(GL_LINE_WIDTH, &lineWidth);
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &polygonOffsetFactor);
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &polygonOffsetUnits);
        glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clearDepth);
    }

    ~PassState()
    {
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        if (scissorEnabled) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (depthEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (blendEnabled) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (polygonOffsetEnabled) glEnable(GL_POLYGON_OFFSET_FILL); else glDisable(GL_POLYGON_OFFSET_FILL);
        glDepthFunc(static_cast<GLenum>(depthFunc));
        glDepthMask(depthMask);
        glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glLineWidth(lineWidth);
        glPolygonOffset(polygonOffsetFactor, polygonOffsetUnits);
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
        : presentation{sourcePresentation}, shader{vertexSource, fragmentSource}, litShader{litVertexSource,litFragmentSource}
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
    std::optional<SolidTarget> solidHover;
    std::optional<SolidTarget> solidSelection;
    const presentation::GeometryPresentation* presentation;
    const presentation::FeaturePresentation* featurePresentation{};
    SolidDisplayMode displayMode{SolidDisplayMode::ShadedWithEdges};
    bool solidPickingEnabled{true};
    ProjectionState projection;
    OrbitNavigation navigation;
    PanZoomNavigation panZoomNavigation;
    NavigationProfile navigationProfile{NavigationProfile::OrbitMiddleButton};
    SelectionAccent selectionAccent{SelectionAccent::Orange};
    bool lastMiddleDown{};
    bool suppressGestureUntilRelease{};
    const math::Scalar verticalFov = std::numbers::pi_v<math::Scalar> / 3.0;
    const math::Scalar nearPlane = 0.1;
    // Keep the origin aids within clipping range at the maximum zoom distance.
    const math::Scalar farPlane = PanZoomNavigation::maximumDistance() + 100.0;
    rendering::ShaderProgram shader;
    rendering::ShaderProgram litShader;
    rendering::TriangleRenderer triangles;
    rendering::LineRenderer solidEdges;
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

void WorkspaceViewport::setPresentation(
    const presentation::GeometryPresentation& presentation) noexcept
{
    if (impl_->presentation == &presentation) return;
    impl_->presentation = &presentation;
    impl_->hover.clear();
    impl_->selection.clear();
    impl_->solidHover.reset(); impl_->solidSelection.reset();
}

void WorkspaceViewport::setFeaturePresentation(const presentation::FeaturePresentation* presentation) noexcept
{ impl_->featurePresentation=presentation; }
void WorkspaceViewport::setDisplayMode(SolidDisplayMode mode) noexcept { impl_->displayMode=mode; }
void WorkspaceViewport::setNavigationProfile(NavigationProfile profile) noexcept
{
    if (impl_->navigationProfile == profile) return;
    impl_->suppressGestureUntilRelease = impl_->lastMiddleDown
        || impl_->navigation.active() || impl_->panZoomNavigation.active();
    impl_->navigation.end();
    impl_->panZoomNavigation.end();
    impl_->navigationProfile = profile;
}
NavigationProfile WorkspaceViewport::navigationProfile() const noexcept { return impl_->navigationProfile; }
void WorkspaceViewport::setSelectionAccent(SelectionAccent accent) noexcept { impl_->selectionAccent = accent; }
SelectionAccent WorkspaceViewport::selectionAccent() const noexcept { return impl_->selectionAccent; }
SolidDisplayMode WorkspaceViewport::displayMode() const noexcept { return impl_->displayMode; }
void WorkspaceViewport::setViewBasis(const math::Vector3& forward, const math::Vector3& up)
{ impl_->camera.setOrientation(forward, up); }
void WorkspaceViewport::setStandardView(StandardView view)
{
    using std::numbers::pi_v; const auto half=pi_v<math::Scalar>/2;
    switch(view){
    case StandardView::Front: impl_->camera.setYaw(-half);impl_->camera.setPitch(0);break;
    case StandardView::Back: impl_->camera.setYaw(half);impl_->camera.setPitch(0);break;
    case StandardView::Left: impl_->camera.setYaw(0);impl_->camera.setPitch(0);break;
    case StandardView::Right: impl_->camera.setYaw(pi_v<math::Scalar>);impl_->camera.setPitch(0);break;
    case StandardView::Top: impl_->camera.setYaw(-half);impl_->camera.setPitch(half);break;
    case StandardView::Bottom: impl_->camera.setYaw(-half);impl_->camera.setPitch(-half);break;
    case StandardView::Isometric: impl_->camera.setYaw(-pi_v<math::Scalar>/4);impl_->camera.setPitch(pi_v<math::Scalar>/6);break;
    }
}
void WorkspaceViewport::setSolidPickingEnabled(bool enabled) noexcept { impl_->solidPickingEnabled=enabled;if(!enabled){impl_->solidHover.reset();impl_->solidSelection.reset();} }
void WorkspaceViewport::fitVisibleGeometry()
{
    if(!impl_->featurePresentation)return; const auto& d=impl_->featurePresentation->solidDisplay();
    if(d.triangles.empty())return; math::Vector3 lo{d.triangles[0].points[0].x(),d.triangles[0].points[0].y(),d.triangles[0].points[0].z()},hi=lo;
    for(const auto&t:d.triangles)for(const auto&p:t.points){lo={std::min(lo.x(),p.x()),std::min(lo.y(),p.y()),std::min(lo.z(),p.z())};hi={std::max(hi.x(),p.x()),std::max(hi.y(),p.y()),std::max(hi.z(),p.z())};}
    const auto extent=std::max<math::Scalar>(1.0,(hi-lo).length());impl_->camera.setTarget((lo+hi)*0.5);impl_->camera.setDistance(extent*1.5);impl_->projection.setVisibleHeight(extent*1.2);
}

void WorkspaceViewport::fitPresentedGeometry(const WorkspaceLayout& layout)
{
    if (!impl_->presentation || impl_->presentation->empty()
        || layout.width <= 0 || layout.height <= 0) return;
    std::vector<math::Vector3> points;
    const auto append = [&](const geometry::Point3& p) {
        points.emplace_back(p.x(), p.y(), p.z());
    };
    for (const auto& entity : impl_->presentation->entities())
        std::visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, geometry::Point3>) append(value);
            else if constexpr (std::is_same_v<T, geometry::Segment3>) {
                append(value.a()); append(value.b());
            } else if constexpr (std::is_same_v<T, geometry::Line3>) append(value.origin());
            else if constexpr (std::is_same_v<T, presentation::Polyline3>)
                for (const auto& point : value.points()) append(point);
            else if constexpr (std::is_same_v<T, presentation::SegmentSet3>)
                for (const auto& segment : value.segments()) {
                    append(segment.a()); append(segment.b());
                }
        }, presentation::worldGeometry(entity));
    if (points.empty()) return;
    auto lo = points.front(), hi = lo;
    for (const auto& p : points) {
        lo = {std::min(lo.x(), p.x()), std::min(lo.y(), p.y()), std::min(lo.z(), p.z())};
        hi = {std::max(hi.x(), p.x()), std::max(hi.y(), p.y()), std::max(hi.z(), p.z())};
    }
    const auto target = (lo + hi) * 0.5;
    const auto right = impl_->camera.right(), up = impl_->camera.up();
    const auto forward = impl_->camera.forward();
    math::Scalar halfWidth = 0, halfHeight = 0, halfDepth = 0;
    for (const auto& p : points) {
        const auto offset = p - target;
        halfWidth = std::max(halfWidth, std::abs(math::dot(offset, right)));
        halfHeight = std::max(halfHeight, std::abs(math::dot(offset, up)));
        halfDepth = std::max(halfDepth, std::abs(math::dot(offset, forward)));
    }
    const auto aspect = layout.width / layout.height;
    const auto visibleHeight = std::max<math::Scalar>(10.0,
        2.8 * std::max(halfHeight, halfWidth / aspect));
    const auto distance = std::max<math::Scalar>(10.0,
        visibleHeight / (2.0 * std::tan(impl_->verticalFov / 2.0)) + halfDepth + 1.0);
    impl_->camera.setTarget(target);
    impl_->camera.setDistance(distance);
    impl_->projection.setVisibleHeight(visibleHeight);
}

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
    impl_->lastMiddleDown = input.middleDown;
    if (!input.middleDown) impl_->suppressGestureUntilRelease = false;
    auto routed = input;
    if (impl_->suppressGestureUntilRelease) routed.middleDown = routed.middlePressed = false;
    const bool alternative = impl_->navigationProfile == NavigationProfile::PanMiddleButton;
    impl_->navigation.handle(layout, routed, impl_->camera, alternative);
    impl_->panZoomNavigation.handle(layout, routed, impl_->camera, impl_->projection, !alternative);
}

std::optional<presentation::VisualEntityId> WorkspaceViewport::hoveredEntity() const noexcept
{
    return impl_->hover.hovered();
}
const std::optional<SolidTarget>& WorkspaceViewport::hoveredSolidTarget() const noexcept{return impl_->solidHover;}
const std::optional<SolidTarget>& WorkspaceViewport::selectedSolidTarget() const noexcept{return impl_->solidSelection;}

std::optional<SolidPickHit> WorkspaceViewport::pickSolidTarget(const WorkspaceLayout& layout,int framebufferWidth,int framebufferHeight,math::Scalar mouseX,math::Scalar mouseY)const
{
    if(!impl_->featurePresentation)return{};const auto rect=framebufferRect(layout,framebufferWidth,framebufferHeight);if(rect.width<=0||rect.height<=0)return{};
    const auto sx=layout.displayWidth/framebufferWidth,sy=layout.displayHeight/framebufferHeight;PickingContext c;c.camera=impl_->camera;c.projection=impl_->projection;c.width=rect.width*sx;c.height=rect.height*sy;c.mouseX=mouseX-rect.x*sx;c.mouseY=mouseY-(framebufferHeight-rect.y-rect.height)*sy;c.projectionAspectRatio=rect.aspectRatio();c.verticalFov=impl_->verticalFov;c.nearPlane=impl_->nearPlane;c.farPlane=impl_->farPlane;
    if(c.mouseX<0||c.mouseX>=c.width||c.mouseY<0||c.mouseY>=c.height)return{};return viewer::pickSolid(impl_->featurePresentation->solidDisplay(),c);
}

void WorkspaceViewport::updateHover(const WorkspaceLayout& layout, const WorkspaceInput& input,
    int framebufferWidth, int framebufferHeight)
{
    // Clear first so unavailable interaction or a failed query cannot leave a
    // stale identity. PickHit and its distance never survive this update.
    impl_->hover.clear();
    impl_->solidHover.reset();
    if (!input.focused || !input.pointerValid || !input.workspaceHovered || input.blocked
        || input.middleDown || input.middlePressed || !contains(layout, input.x, input.y))
        return;
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0)
        return; // Normal minimize/resize frames must not call the strict picker.
    if(impl_->solidPickingEnabled)if(const auto hit=pickSolidTarget(layout,framebufferWidth,framebufferHeight,input.x,input.y)){impl_->solidHover=hit->target;return;}
    if (const auto hit = pick(layout, framebufferWidth, framebufferHeight, input.x, input.y))
        impl_->hover.update(hit->id);
}

std::optional<presentation::VisualEntityId> WorkspaceViewport::selectedEntity() const noexcept
{
    return impl_->selection.selected();
}

void WorkspaceViewport::selectEntity(std::optional<presentation::VisualEntityId> id) noexcept
{
    impl_->solidSelection.reset();
    if (id) impl_->selection.select(*id); else impl_->selection.clear();
}

void WorkspaceViewport::clearSelection() noexcept { impl_->selection.clear();impl_->solidSelection.reset(); }

std::optional<geometry::Point3> WorkspaceViewport::pointOnGlobalXY(
    const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
    math::Scalar mouseX, math::Scalar mouseY) const
{
    return pointOnPlane(layout, framebufferWidth, framebufferHeight, mouseX, mouseY,
        geometry::Plane{{}, {0, 0, 1}});
}

std::optional<geometry::Point3> WorkspaceViewport::pointOnPlane(
    const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
    math::Scalar mouseX, math::Scalar mouseY, const geometry::Plane& plane) const
{
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0) return std::nullopt;
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
    context.verticalFov = impl_->verticalFov;
    context.nearPlane = impl_->nearPlane;
    context.farPlane = impl_->farPlane;
    if (context.mouseX < 0 || context.mouseX >= context.width
        || context.mouseY < 0 || context.mouseY >= context.height) return std::nullopt;
    const auto ray = makePickingRay(context);
    const auto denominator=math::dot(ray.direction(),plane.normal());
    if (std::abs(denominator) <= geometry::defaultGeometricTolerance) return std::nullopt;
    const auto t=math::dot(plane.origin()-ray.origin(),plane.normal())/denominator;
    if (t < 0 || !std::isfinite(t)) return std::nullopt;
    return ray.pointAt(t);
}

std::optional<ScreenPoint> WorkspaceViewport::projectPoint(
    const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
    const geometry::Point3& point) const
{
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0) return std::nullopt;
    const auto logicalPerPixelX = layout.displayWidth / framebufferWidth;
    const auto logicalPerPixelY = layout.displayHeight / framebufferHeight;
    PickingContext context;
    context.camera = impl_->camera;
    context.projection = impl_->projection;
    context.width = rect.width * logicalPerPixelX;
    context.height = rect.height * logicalPerPixelY;
    context.mouseX = 0;
    context.mouseY = 0;
    context.projectionAspectRatio = rect.aspectRatio();
    context.verticalFov = impl_->verticalFov;
    context.nearPlane = impl_->nearPlane;
    context.farPlane = impl_->farPlane;
    return projectWorldToScreen(point, context);
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
    if(impl_->solidPickingEnabled)if(const auto hit=pickSolidTarget(layout,framebufferWidth,framebufferHeight,input.x,input.y)){impl_->solidSelection=hit->target;impl_->selection.clear();return;}
    impl_->solidSelection.reset();
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
        else if constexpr (std::is_same_v<Geometry, geometry::Line3>) check(value.origin());
        else if constexpr (std::is_same_v<Geometry, presentation::Polyline3>)
            for (const auto& point : value.points()) check(point);
        else for (const auto& segment : value.segments()) { check(segment.a()); check(segment.b()); }
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
    glDisable(GL_POLYGON_OFFSET_FILL);
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

    if(impl_->featurePresentation)
    {
        const auto& display=impl_->featurePresentation->solidDisplay();
        const auto passes=displayPasses(impl_->displayMode);
        if(passes.faces)
        {
            std::vector<rendering::LitVertex> vertices;vertices.reserve(display.triangles.size()*3);
            for(const auto&t:display.triangles)for(const auto&p:t.points)vertices.push_back({{p.x(),p.y(),p.z()},t.normal});
            impl_->triangles.setVertices(vertices);impl_->litShader.bind();impl_->litShader.setMatrix4("uModel",math::Matrix4::identity());impl_->litShader.setMatrix4("uView",viewMatrix(impl_->camera));impl_->litShader.setMatrix4("uProjection",impl_->projection.matrix(impl_->verticalFov,rect.aspectRatio(),impl_->nearPlane,impl_->farPlane));impl_->litShader.setVector3("uColor",{0.52,0.64,0.78});
            // Bias filled triangles behind their own CAD edges so a pocket-floor
            // boundary remains visible without exposing edges behind the solid.
            glPolygonOffset(1.0F,1.0F);glEnable(GL_POLYGON_OFFSET_FILL);
            impl_->triangles.draw();
            glDisable(GL_POLYGON_OFFSET_FILL);
            impl_->shader.bind();impl_->shader.setMatrix4("uModel",math::Matrix4::identity());impl_->shader.setMatrix4("uView",viewMatrix(impl_->camera));impl_->shader.setMatrix4("uProjection",impl_->projection.matrix(impl_->verticalFov,rect.aspectRatio(),impl_->nearPlane,impl_->farPlane));
        }
        if(passes.topologyEdges)
        {
            std::vector<math::Vector3> vertices;for(const auto&e:display.edges)for(std::size_t i=1;i<e.points.size();++i){const auto&a=e.points[i-1];const auto&b=e.points[i];vertices.push_back({a.x(),a.y(),a.z()});vertices.push_back({b.x(),b.y(),b.z()});}
            impl_->solidEdges.setVertices(vertices);impl_->shader.setVector3("uColor",topologyEdgeColor(impl_->displayMode));glDepthFunc(GL_LEQUAL);impl_->solidEdges.draw();glDepthFunc(GL_LESS);
        }
        const auto drawState=[&](const std::optional<SolidTarget>& target,const math::Vector3& color){
            if(!target)return;
            const auto* face=std::get_if<interaction::TopologyFaceTarget>(&*target);
            const auto region=face?presentation::faceHighlightRegion(display,*face)
                :std::vector<interaction::TopologyFaceTarget>{};
            const auto* pickedEdge=std::get_if<interaction::TopologyEdgeTarget>(&*target);
            const auto edgeRegion=pickedEdge?presentation::edgeHighlightRegion(display,*pickedEdge)
                :std::vector<interaction::TopologyEdgeTarget>{};
            if(face&&passes.faces)
            {
                std::vector<rendering::LitVertex> vertices;
                for(const auto& triangle:display.triangles)
                    if(std::find(region.begin(),region.end(),triangle.target)!=region.end())
                        for(const auto& point:triangle.points)
                            vertices.push_back({{point.x(),point.y(),point.z()},triangle.normal});
                impl_->triangles.setVertices(vertices);
                impl_->litShader.bind();
                impl_->litShader.setMatrix4("uModel",math::Matrix4::identity());
                impl_->litShader.setMatrix4("uView",viewMatrix(impl_->camera));
                impl_->litShader.setMatrix4("uProjection",impl_->projection.matrix(
                    impl_->verticalFov,rect.aspectRatio(),impl_->nearPlane,impl_->farPlane));
                impl_->litShader.setVector3("uColor",color);
                glDepthFunc(GL_LEQUAL);
                impl_->triangles.draw();
                glDepthFunc(GL_LESS);
                return;
            }
            std::vector<math::Vector3> vertices;
            for(const auto& edge:display.edges)
            {
                const bool selectedFace=face&&std::any_of(edge.incidentFaces.begin(),
                    edge.incidentFaces.end(),[&](const auto& incident){
                        return std::find(region.begin(),region.end(),incident)!=region.end();});
                const bool selectedEdge=pickedEdge
                    &&std::find(edgeRegion.begin(),edgeRegion.end(),edge.target)!=edgeRegion.end();
                if(!selectedFace&&!selectedEdge)continue;
                for(std::size_t i=1;i<edge.points.size();++i)
                {
                    vertices.push_back({edge.points[i-1].x(),edge.points[i-1].y(),edge.points[i-1].z()});
                    vertices.push_back({edge.points[i].x(),edge.points[i].y(),edge.points[i].z()});
                }
            }
            impl_->solidEdges.setVertices(vertices);
            impl_->shader.bind();
            impl_->shader.setMatrix4("uModel",math::Matrix4::identity());
            impl_->shader.setMatrix4("uView",viewMatrix(impl_->camera));
            impl_->shader.setMatrix4("uProjection",impl_->projection.matrix(
                impl_->verticalFov,rect.aspectRatio(),impl_->nearPlane,impl_->farPlane));
            impl_->shader.setVector3("uColor",color);
            glDepthFunc(GL_LEQUAL);
            if(pickedEdge)glLineWidth(3.0F);
            impl_->solidEdges.draw();
            if(pickedEdge)glLineWidth(saved.lineWidth);
            glDepthFunc(GL_LESS);
        };
        drawState(impl_->solidHover,{0.25,0.9,1.0});
        drawState(impl_->solidSelection,selectedSolidHighlightColor(impl_->selectionAccent));
        // A selected feature has a logical VisualEntityId but no explicit
        // face/edge target. Trace its topology boundary in every display mode.
        if (impl_->selection.selected())
            if (const auto selectedFeature = impl_->featurePresentation->featureId(*impl_->selection.selected()))
            {
                std::vector<math::Vector3> outline;
                for (const auto& edge : display.edges)
                    if (edge.target.feature == *selectedFeature)
                        for (std::size_t i = 1; i < edge.points.size(); ++i)
                        {
                            const auto& a = edge.points[i-1]; const auto& b = edge.points[i];
                            outline.push_back({a.x(), a.y(), a.z()});
                            outline.push_back({b.x(), b.y(), b.z()});
                        }
                impl_->solidEdges.setVertices(outline);
                impl_->shader.bind();
                impl_->shader.setMatrix4("uModel",math::Matrix4::identity());
                impl_->shader.setMatrix4("uView",viewMatrix(impl_->camera));
                impl_->shader.setMatrix4("uProjection",impl_->projection.matrix(
                    impl_->verticalFov,rect.aspectRatio(),impl_->nearPlane,impl_->farPlane));
                impl_->shader.setVector3("uColor",selectedSolidHighlightColor(impl_->selectionAccent));
                glDepthFunc(GL_LEQUAL); impl_->solidEdges.draw(); glDepthFunc(GL_LESS);
            }
        // Shaded face feedback covers connected coplanar patches without
        // changing the picked FaceId. Wireframe uses visible CAD edges only.
        impl_->shader.bind();
        impl_->shader.setMatrix4("uModel",math::Matrix4::identity());
        impl_->shader.setMatrix4("uView",viewMatrix(impl_->camera));
        impl_->shader.setMatrix4("uProjection",impl_->projection.matrix(
            impl_->verticalFov,rect.aspectRatio(),impl_->nearPlane,impl_->farPlane));
        // Visible sketches remain as subdued CAD references after their feature
        // has been consumed. They are intentionally excluded from picking here;
        // editing activates the dedicated SketchPresentation instead.
        impl_->segments.setVertices(presentedSegmentVertices(
            impl_->featurePresentation->sketchReferences()));
        impl_->shader.setVector3("uColor",{0.32,0.42,0.48});
        glDepthFunc(GL_LEQUAL);impl_->segments.draw();glDepthFunc(GL_LESS);
    }

    // Reuse three generic GPU buffers for successive state batches. IDs stay on CPU.
    const auto upload = [&](VisualState state)
    {
        if (!impl_->presentation) return;
        const bool solidWirePresentation = impl_->featurePresentation
            && impl_->presentation == &impl_->featurePresentation->geometry();
        // FeaturePresentation::geometry() is only the legacy visual proxy used
        // for feature identity. Solid faces, topology edges and their hover /
        // selection feedback are rendered by the explicit solid passes above.
        // Drawing this proxy in any visual state would leak the complete solid
        // wireframe into Shaded mode whenever its feature is selected.
        if (solidWirePresentation)
        {
            impl_->lines.setVertices({});
            impl_->segments.setVertices({});
            impl_->points.setVertices({});
            return;
        }
        const VisualStateFilter filter{state, impl_->hover.hovered(), impl_->selection.selected()};
        impl_->lines.setVertices(presentedLineVertices(*impl_->presentation, lineContext, filter));
        impl_->segments.setVertices(presentedSegmentVertices(*impl_->presentation, filter));
        impl_->points.setVertices(presentedPointVertices(*impl_->presentation, filter));
    };
    const auto drawLines = [&](VisualState state)
    {
        impl_->shader.setVector3("uColor", highlightColor(state, normalLineColor, impl_->selectionAccent));
        impl_->lines.draw();
    };
    const auto drawSegmentsAndPoints = [&](VisualState state)
    {
        impl_->shader.setVector3("uColor", highlightColor(state, normalSegmentColor, impl_->selectionAccent));
        impl_->segments.draw();
        impl_->shader.setVector3("uColor", highlightColor(state, normalPointColor, impl_->selectionAccent));
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
