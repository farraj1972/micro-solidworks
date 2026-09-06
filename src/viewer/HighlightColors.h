#pragma once

#include "core/math/Vector3.h"
#include "viewer/VisualState.h"

namespace microsw::viewer
{
// Fixed B4 appearance policy. Normal colors retain the pre-highlight palette.
inline const math::Vector3 normalPointColor{1.0, 0.85, 0.2};
inline const math::Vector3 normalSegmentColor{0.8, 0.8, 0.85};
inline const math::Vector3 normalLineColor{0.2, 0.75, 0.85};
inline const math::Vector3 hoveredGeometryColor{0.65, 1.0, 1.0};
inline const math::Vector3 selectedGeometryColor{1.0, 0.4, 0.05};

[[nodiscard]] inline const math::Vector3& highlightColor(
    VisualState state, const math::Vector3& normalColor) noexcept
{
    if (state == VisualState::Selected) return selectedGeometryColor;
    if (state == VisualState::Hovered) return hoveredGeometryColor;
    return normalColor;
}
}
