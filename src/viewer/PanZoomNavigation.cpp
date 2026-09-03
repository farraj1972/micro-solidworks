#include "viewer/PanZoomNavigation.h"

#include "viewer/OrbitNavigation.h"

#include <cmath>

namespace microsw::viewer
{
void PanZoomNavigation::handle(
    const WorkspaceLayout& workspace, const WorkspaceInput& input, OrbitCamera& camera)
{
    if (!input.focused || !input.pointerValid || input.blocked
        || !std::isfinite(input.x) || !std::isfinite(input.y))
    {
        active_ = false;
        return;
    }
    const bool overWorkspace = input.workspaceHovered && contains(workspace, input.x, input.y);
    if (!input.middleDown || !input.shiftDown)
        active_ = false;
    else if (input.middlePressed)
    {
        active_ = overWorkspace;
        previousX_ = input.x;
        previousY_ = input.y; // Anchor only; no first-frame movement.
    }
    else if (active_)
        updatePan(input.x, input.y, camera);

    // Wheel is independent of drag capture and always requires hover permission.
    if (overWorkspace)
        zoom(input.wheelDelta, camera);
}

void PanZoomNavigation::updatePan(double x, double y, OrbitCamera& camera)
{
    const double scale = camera.distance() * panSensitivity();
    // Scene follows the cursor; UI Y grows down. Only target is translated.
    const auto delta = camera.right() * (-(x - previousX_) * scale)
        + camera.up() * ((y - previousY_) * scale);
    const auto target = camera.target() + delta;
    if (!std::isfinite(target.x()) || !std::isfinite(target.y()) || !std::isfinite(target.z()))
    {
        active_ = false; // Invalid/overflowing motion cancels without partial mutation.
        return;
    }
    camera.setTarget(target);
    previousX_ = x;
    previousY_ = y;
}

void PanZoomNavigation::zoom(double wheelDelta, OrbitCamera& camera)
{
    if (!std::isfinite(wheelDelta) || wheelDelta == 0.0)
        return;
    // Equivalent to distance * exp(-wheel * sensitivity), evaluated in log
    // space so extreme input clamps before exponentiation can overflow/underflow.
    const double logDistance = std::log(camera.distance()) - wheelDelta * zoomSensitivity();
    if (logDistance <= std::log(minimumDistance()))
        camera.setDistance(minimumDistance());
    else if (logDistance >= std::log(maximumDistance()))
        camera.setDistance(maximumDistance());
    else
        camera.setDistance(std::exp(logDistance));
}
}
