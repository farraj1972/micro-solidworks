#pragma once

#include "app/WorkspaceLayout.h"
#include "core/math/Scalar.h"

#include <memory>

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
    ~WorkspaceViewport();
    WorkspaceViewport(const WorkspaceViewport&) = delete;
    WorkspaceViewport& operator=(const WorkspaceViewport&) = delete;

    // Empty/invalid surfaces are no-ops. Restores the OpenGL states changed by this pass.
    void render(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
