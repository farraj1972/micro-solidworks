#pragma once

#include "app/WorkspaceInput.h"
#include "app/WorkspaceLayout.h"
#include "viewer/OrbitCamera.h"

namespace microsw::viewer
{
// Logical display coordinates, clipped to display; left/top inclusive,
// right/bottom exclusive. Invalid/empty layouts never contain a point.
[[nodiscard]] bool contains(const WorkspaceLayout& workspace, double x, double y) noexcept;

class OrbitNavigation
{
public:
    // Implementation parameter, radians per logical UI pixel.
    [[nodiscard]] static constexpr double sensitivity() noexcept { return 0.005; }
    void begin(double x, double y) noexcept;
    // Inactive updates do nothing. Invalid coordinates/overflow cancel safely.
    void update(double x, double y, OrbitCamera& camera);
    void end() noexcept { active_ = false; }
    [[nodiscard]] bool active() const noexcept { return active_; }
    void handle(const WorkspaceLayout& workspace, const WorkspaceInput& input, OrbitCamera& camera);

private:
    bool active_{};
    double previousX_{}, previousY_{};
};
}
