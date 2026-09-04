#pragma once

#include "app/ProjectionMode.h"
#include "core/math/Matrix4.h"

namespace microsw::viewer
{
// Projection policy/state only: never changes or owns camera pose/distance.
class ProjectionState
{
public:
    [[nodiscard]] ProjectionMode mode() const noexcept { return mode_; }
    void setMode(ProjectionMode mode) noexcept { mode_ = mode; }
    [[nodiscard]] math::Scalar visibleHeight() const noexcept { return visibleHeight_; }
    // Rejects non-finite/non-positive heights with invalid_argument.
    void setVisibleHeight(math::Scalar height);
    [[nodiscard]] math::Matrix4 matrix(
        math::Scalar verticalFov, math::Scalar aspect,
        math::Scalar nearPlane, math::Scalar farPlane) const;

private:
    ProjectionMode mode_{ProjectionMode::Perspective};
    math::Scalar visibleHeight_{10.0}; // Independent implementation default.
};
}
