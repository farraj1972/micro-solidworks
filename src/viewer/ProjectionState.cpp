#include "viewer/ProjectionState.h"
#include "viewer/ViewProjection.h"

#include <cmath>
#include <stdexcept>

namespace microsw::viewer
{
void ProjectionState::setVisibleHeight(math::Scalar height)
{
    if (!std::isfinite(height) || height <= 0.0)
        throw std::invalid_argument{"Orthographic visible height must be positive and finite"};
    visibleHeight_ = height;
}

math::Matrix4 ProjectionState::matrix(
    math::Scalar verticalFov, math::Scalar aspect,
    math::Scalar nearPlane, math::Scalar farPlane) const
{
    return mode_ == ProjectionMode::Perspective
        ? perspective(verticalFov, aspect, nearPlane, farPlane)
        : orthographic(visibleHeight_, aspect, nearPlane, farPlane);
}
}
