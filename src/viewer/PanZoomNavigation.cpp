#include "viewer/PanZoomNavigation.h"

#include "viewer/OrbitNavigation.h"
#include "viewer/ProjectionState.h"

#include <cmath>

namespace microsw::viewer
{
void PanZoomNavigation::handle(
    const WorkspaceLayout& workspace, const WorkspaceInput& input, OrbitCamera& camera)
{
    ProjectionState perspectiveState;
    handle(workspace, input, camera, perspectiveState);
}

void PanZoomNavigation::handle(
    const WorkspaceLayout& workspace, const WorkspaceInput& input,
    OrbitCamera& camera, ProjectionState& projection)
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
        updatePan(input.x, input.y, camera, projection.mode() == ProjectionMode::Perspective
            ? camera.distance() : projection.visibleHeight());

    // Wheel is independent of drag capture and always requires hover permission.
    if (overWorkspace)
    {
        if (projection.mode() == ProjectionMode::Perspective)
            camera.setDistance(zoomedScale(input.wheelDelta, camera.distance(),
                minimumDistance(), maximumDistance()));
        else
            projection.setVisibleHeight(zoomedScale(input.wheelDelta, projection.visibleHeight(),
                minimumVisibleHeight(), maximumVisibleHeight()));
    }
}

void PanZoomNavigation::updatePan(double x, double y, OrbitCamera& camera, double referenceScale)
{
    const double scale = referenceScale * panSensitivity();
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

double PanZoomNavigation::zoomedScale(double wheelDelta, double current, double minimum, double maximum)
{
    if (!std::isfinite(wheelDelta) || wheelDelta == 0.0)
        return current;
    // Equivalent to distance * exp(-wheel * sensitivity), evaluated in log
    // space so extreme input clamps before exponentiation can overflow/underflow.
    const double logScale = std::log(current) - wheelDelta * zoomSensitivity();
    if (logScale <= std::log(minimum))
        return minimum;
    if (logScale >= std::log(maximum))
        return maximum;
    return std::exp(logScale);
}
}
