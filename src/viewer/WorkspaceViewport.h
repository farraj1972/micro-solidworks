#pragma once

#include "app/WorkspaceLayout.h"
#include "app/WorkspaceInput.h"
#include "core/math/Scalar.h"
#include "core/math/Vector3.h"
#include "core/geometry/Plane.h"
#include "viewer/GeometryPicker.h"
#include "viewer/SolidPicker.h"
#include "viewer/ViewerPreferences.h"

#include <memory>

namespace microsw::presentation { class GeometryPresentation; class FeaturePresentation; }
namespace microsw::math { class Transform3; }

namespace microsw::viewer
{
enum class SolidDisplayMode { ShadedWithEdges, Wireframe, Shaded };
struct SolidDisplayPasses { bool faces; bool topologyEdges; };
[[nodiscard]] constexpr SolidDisplayPasses displayPasses(SolidDisplayMode mode) noexcept
{
    return {mode != SolidDisplayMode::Wireframe, mode != SolidDisplayMode::Shaded};
}
[[nodiscard]] constexpr math::Vector3 topologyEdgeColor(SolidDisplayMode mode) noexcept
{
    // Dark edges read against lit faces; Wireframe needs light edges because it
    // draws directly over the dark workspace background.
    return mode == SolidDisplayMode::Wireframe
        ? math::Vector3{0.72, 0.78, 0.86}
        : math::Vector3{0.08, 0.10, 0.13};
}
enum class StandardView { Front, Back, Left, Right, Top, Bottom, Isometric };

struct ViewportRect
{
    int x{};
    int y{};
    int width{};
    int height{};
    [[nodiscard]] math::Scalar aspectRatio() const noexcept;
};

// Pure conversion: HiDPI scaling, framebuffer clipping and top-left to bottom-left.
// Pixel bounds round inward; invalid/empty/outside layouts yield an empty rect.
[[nodiscard]] ViewportRect framebufferRect(
    const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight);

// Owns view state and GPU resources, not CAD entities. Requires current GL context
// and loaded GLAD throughout lifetime; destroy before the window/context.
class WorkspaceViewport
{
public:
    WorkspaceViewport();
    explicit WorkspaceViewport(const presentation::GeometryPresentation& presentation);
    ~WorkspaceViewport();
    WorkspaceViewport(const WorkspaceViewport&) = delete;
    WorkspaceViewport& operator=(const WorkspaceViewport&) = delete;
    // Switching model sources clears interaction identities from the old source.
    void setPresentation(const presentation::GeometryPresentation& presentation) noexcept;
    void setFeaturePresentation(const presentation::FeaturePresentation* presentation) noexcept;
    void setSolidPickingEnabled(bool enabled) noexcept;
    void setDisplayMode(SolidDisplayMode mode) noexcept;
    void setNavigationProfile(NavigationProfile profile) noexcept;
    [[nodiscard]] NavigationProfile navigationProfile() const noexcept;
    void setSelectionAccent(SelectionAccent accent) noexcept;
    [[nodiscard]] SelectionAccent selectionAccent() const noexcept;
    [[nodiscard]] SolidDisplayMode displayMode() const noexcept;
    void setViewBasis(const math::Vector3& forward, const math::Vector3& up);
    void setStandardView(StandardView view);
    void fitVisibleGeometry();
    // Frame the current derived geometry in the existing camera orientation.
    void fitPresentedGeometry(const WorkspaceLayout& layout);

    // Empty/invalid surfaces are no-ops. Restores the OpenGL states changed by this pass.
    void render(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight);
    void updateNavigation(const WorkspaceLayout& layout, const WorkspaceInput& input);
    // Call after updateNavigation with the same frame's input and framebuffer.
    // Suppress hover while MMB is held (orbit/pan), or interaction is unavailable.
    void updateHover(const WorkspaceLayout& layout, const WorkspaceInput& input,
        int framebufferWidth, int framebufferHeight);
    [[nodiscard]] std::optional<presentation::VisualEntityId> hoveredEntity() const noexcept;
    [[nodiscard]] const std::optional<SolidTarget>& hoveredSolidTarget() const noexcept;
    // Call after navigation/hover. Only an eligible leftPressed changes selection.
    void updateSelection(const WorkspaceLayout& layout, const WorkspaceInput& input,
        int framebufferWidth, int framebufferHeight);
    [[nodiscard]] std::optional<presentation::VisualEntityId> selectedEntity() const noexcept;
    [[nodiscard]] const std::optional<SolidTarget>& selectedSolidTarget() const noexcept;
    void selectEntity(std::optional<presentation::VisualEntityId> id) noexcept;
    void clearSelection() noexcept;
    // Preflight a UI edit against world Geometry and the renderer's numeric range.
    // Does not mutate Presentation or interaction state.
    void validateTransform(presentation::VisualEntityId id, const math::Transform3& transform) const;
    [[nodiscard]] ProjectionMode projectionMode() const noexcept;
    void setProjectionMode(ProjectionMode mode) noexcept;

    // Read-only query. Mouse uses the main viewport's logical coordinates,
    // like WorkspaceInput; converted to the effective raster viewport in logical
    // units. Supply the same framebuffer size as render (including HiDPI).
    // This query does not change interaction state. Invalid layout
    // or non-finite mouse throws invalid_argument; outside returns no hit.
    [[nodiscard]] std::optional<PickHit> pick(
        const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
        math::Scalar mouseX, math::Scalar mouseY) const;
    [[nodiscard]] std::optional<SolidPickHit> pickSolidTarget(
        const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
        math::Scalar mouseX, math::Scalar mouseY) const;
    [[nodiscard]] std::optional<geometry::Point3> pointOnGlobalXY(
        const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
        math::Scalar mouseX, math::Scalar mouseY) const;
    [[nodiscard]] std::optional<geometry::Point3> pointOnPlane(
        const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
        math::Scalar mouseX, math::Scalar mouseY, const geometry::Plane& plane) const;
    [[nodiscard]] std::optional<ScreenPoint> projectPoint(
        const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
        const geometry::Point3& point) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
