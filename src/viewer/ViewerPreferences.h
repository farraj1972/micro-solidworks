#pragma once

namespace microsw::viewer
{
// Typed input and display choices consumed by Viewer. Application owns their
// persistence; Viewer only receives the current values.
enum class NavigationProfile { OrbitMiddleButton, PanMiddleButton };
enum class SelectionAccent { Orange, Blue };
}
