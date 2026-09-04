#pragma once

namespace microsw
{
// Shared application contract for UI commands/indication, without UI -> Viewer
// linkage. The selected mode and scale remain owned by the viewer.
enum class ProjectionMode
{
    Perspective,
    Orthographic
};
}
