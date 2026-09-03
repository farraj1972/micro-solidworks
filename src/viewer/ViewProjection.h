#pragma once

#include "core/math/Matrix4.h"

namespace microsw::viewer
{

class OrbitCamera;

// World to camera space, with basis (right, up, -forward).
[[nodiscard]] math::Matrix4 viewMatrix(const OrbitCamera& camera);

// Vertical FOV in radians (0, pi), aspect = width / height.
// Both projections use positive near/far distances, 0 < near < far,
// and map view-space -near/-far to NDC depth -1/+1.
// Invalid or non-finite parameters throw std::invalid_argument.
// Unrepresentable matrix coefficients throw std::overflow_error.
[[nodiscard]] math::Matrix4 perspective(
    math::Scalar verticalFovRadians, math::Scalar aspectRatio,
    math::Scalar nearPlane, math::Scalar farPlane);

// Symmetric bounds: visibleWidth = visibleHeight * aspectRatio.
// visibleHeight and aspectRatio must be positive.
[[nodiscard]] math::Matrix4 orthographic(
    math::Scalar visibleHeight, math::Scalar aspectRatio,
    math::Scalar nearPlane, math::Scalar farPlane);

}
