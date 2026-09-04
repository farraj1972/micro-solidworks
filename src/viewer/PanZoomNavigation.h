#pragma once

#include "app/WorkspaceInput.h"
#include "app/WorkspaceLayout.h"
#include "viewer/OrbitCamera.h"

namespace microsw::viewer
{
class ProjectionState;
// Image-plane pan and mode-specific zoom, independent of UI/GPU bindings.
class PanZoomNavigation
{
public:
    // Implementation/UX parameters, not mathematical camera constraints.
    [[nodiscard]] static constexpr double panSensitivity() noexcept { return 0.0015; }
    [[nodiscard]] static constexpr double zoomSensitivity() noexcept { return 0.15; }
    [[nodiscard]] static constexpr double minimumDistance() noexcept { return 0.1; }
    [[nodiscard]] static constexpr double maximumDistance() noexcept { return 1000.0; }
    [[nodiscard]] static constexpr double minimumVisibleHeight() noexcept { return 0.1; }
    [[nodiscard]] static constexpr double maximumVisibleHeight() noexcept { return 1000.0; }

    // Perspective-only entry point preserves the B2.9 contract.
    void handle(const WorkspaceLayout& workspace, const WorkspaceInput& input, OrbitCamera& camera);
    void handle(const WorkspaceLayout& workspace, const WorkspaceInput& input,
                OrbitCamera& camera, ProjectionState& projection);
    [[nodiscard]] bool active() const noexcept { return active_; }

private:
    void updatePan(double x, double y, OrbitCamera& camera, double referenceScale);
    // Non-finite wheel input is ignored; finite extreme input saturates.
    static double zoomedScale(double wheelDelta, double current, double minimum, double maximum);
    bool active_{};
    double previousX_{}, previousY_{};
};
}
