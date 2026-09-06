#pragma once

#include "viewer/OrbitCamera.h"
#include "viewer/ProjectionState.h"

#include <numbers>
#include <optional>

namespace microsw::viewer
{
// Interaction tolerance in logical pixels, independent of Geometry tolerance.
inline constexpr math::Scalar defaultPickingTolerancePixels = 6.0;

// A value snapshot, with no interaction state or graphics resources.
// Mouse is Workspace-local: top-left origin, +X right, +Y down.
// Width, height and tolerance use logical pixels, never framebuffer pixels.
struct PickingContext
{
    OrbitCamera camera;
    ProjectionState projection;
    math::Scalar width{};
    math::Scalar height{};
    math::Scalar mouseX{};
    math::Scalar mouseY{};
    math::Scalar verticalFov = std::numbers::pi_v<math::Scalar> / 3.0;
    math::Scalar nearPlane = 0.1;
    math::Scalar farPlane = 1100.0;
    math::Scalar tolerancePixels = defaultPickingTolerancePixels;
    // Normally width/height. Workspace supplies the actual raster aspect when
    // framebuffer scaling is nonuniform; input and distances stay logical.
    std::optional<math::Scalar> projectionAspectRatio;

    [[nodiscard]] math::Scalar aspectRatio() const noexcept
    {
        return projectionAspectRatio.value_or(width / height);
    }

    // Invalid inputs throw invalid_argument; unrepresentable derived
    // projection coefficients throw overflow_error. Outside mouse is valid.
    void validate() const;
};
}
