#pragma once

#include "app/ProjectionMode.h"
#include <optional>

namespace microsw
{
// Per-frame UI snapshot in the same logical coordinates as WorkspaceLayout.
// UI capture decisions stay at the UI boundary; no library-specific types escape.
struct WorkspaceInput
{
    double x{}, y{};
    bool middlePressed{};
    bool middleDown{};
    bool shiftDown{};
    bool focused{};
    bool pointerValid{};
    bool workspaceHovered{};
    bool blocked{};
    double wheelDelta{};
    // One-frame UI request; absence means preserve the viewer's current mode.
    std::optional<ProjectionMode> projectionRequest{};
};
}
