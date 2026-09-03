#pragma once

namespace microsw
{
// UI layout units, top-left origin relative to the main application viewport.
// Display dimensions use the same logical units, not framebuffer pixels.
struct WorkspaceLayout
{
    double x{};
    double y{};
    double width{};
    double height{};
    double displayWidth{};
    double displayHeight{};
};
}
