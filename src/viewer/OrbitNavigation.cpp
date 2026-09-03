#include "viewer/OrbitNavigation.h"

#include <cmath>

namespace microsw::viewer
{
bool contains(const WorkspaceLayout& workspace, double x, double y) noexcept
{
    for (double value : {workspace.x, workspace.y, workspace.width, workspace.height,
                         workspace.displayWidth, workspace.displayHeight, x, y})
        if (!std::isfinite(value)) return false;
    if (workspace.width <= 0 || workspace.height <= 0
        || workspace.displayWidth <= 0 || workspace.displayHeight <= 0)
        return false;
    // Differences avoid overflowing x + width on extreme layouts.
    return x >= 0 && y >= 0 && x < workspace.displayWidth && y < workspace.displayHeight
        && x >= workspace.x && y >= workspace.y
        && x - workspace.x < workspace.width && y - workspace.y < workspace.height;
}

void OrbitNavigation::begin(double x, double y) noexcept
{
    active_ = std::isfinite(x) && std::isfinite(y);
    previousX_ = x;
    previousY_ = y;
}

void OrbitNavigation::update(double x, double y, OrbitCamera& camera)
{
    if (!active_) return;
    const double yaw = camera.yaw() + (x - previousX_) * sensitivity();
    // UI Y grows down: dragging up increases elevation.
    const double pitch = camera.pitch() - (y - previousY_) * sensitivity();
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(yaw) || !std::isfinite(pitch))
    {
        end();
        return;
    }
    camera.setYaw(yaw); // Deliberately unwrapped.
    camera.setPitch(pitch); // Only OrbitCamera owns pole protection.
    previousX_ = x;
    previousY_ = y;
}

void OrbitNavigation::handle(
    const WorkspaceLayout& workspace, const WorkspaceInput& input, OrbitCamera& camera)
{
    if (!input.focused || !input.pointerValid || input.blocked
        || input.shiftDown || !input.middleDown)
    {
        end();
        return;
    }
    if (input.middlePressed)
    {
        end();
        if (input.workspaceHovered && contains(workspace, input.x, input.y))
            begin(input.x, input.y); // First frame stores position; never moves camera.
    }
    else
    {
        // A captured gesture may leave the Workspace until release/cancellation.
        update(input.x, input.y, camera);
    }
}
}
