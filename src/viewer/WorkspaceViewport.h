#pragma once

#include "app/WorkspaceLayout.h"
#include "app/WorkspaceInput.h"
#include "core/math/Scalar.h"
#include "viewer/GeometryPicker.h"

#include <memory>

namespace microsw::presentation { class GeometryPresentation; }

namespace microsw::viewer
{

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

    // Empty/invalid surfaces are no-ops. Restores the OpenGL states changed by this pass.
    void render(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight);
    void updateNavigation(const WorkspaceLayout& layout, const WorkspaceInput& input);
    [[nodiscard]] ProjectionMode projectionMode() const noexcept;
    void setProjectionMode(ProjectionMode mode) noexcept;

    // Read-only query. Mouse uses the main viewport's logical coordinates,
    // like WorkspaceInput; converted to the effective raster viewport in logical
    // units. Supply the same framebuffer size as render (including HiDPI).
    // No event integration or stored hover/selection state. Invalid layout
    // or non-finite mouse throws invalid_argument; outside returns no hit.
    [[nodiscard]] std::optional<PickHit> pick(
        const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight,
        math::Scalar mouseX, math::Scalar mouseY) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
