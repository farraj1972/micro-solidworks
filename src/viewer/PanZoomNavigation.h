#pragma once

#include "app/WorkspaceInput.h"
#include "app/WorkspaceLayout.h"
#include "viewer/OrbitCamera.h"

namespace microsw::viewer
{
// Image-plane pan and perspective-distance zoom, independent of UI/GPU bindings.
class PanZoomNavigation
{
public:
    // Implementation/UX parameters, not mathematical camera constraints.
    [[nodiscard]] static constexpr double panSensitivity() noexcept { return 0.0015; }
    [[nodiscard]] static constexpr double zoomSensitivity() noexcept { return 0.15; }
    [[nodiscard]] static constexpr double minimumDistance() noexcept { return 0.1; }
    [[nodiscard]] static constexpr double maximumDistance() noexcept { return 1000.0; }

    void handle(const WorkspaceLayout& workspace, const WorkspaceInput& input, OrbitCamera& camera);
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    void updatePan(double x, double y, OrbitCamera& camera);
    // Non-finite wheel input is ignored; finite extreme input saturates.
    void zoom(double wheelDelta, OrbitCamera& camera);
    bool active_{};
    double previousX_{}, previousY_{};
};
}
