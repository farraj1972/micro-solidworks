#pragma once

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
};
}
