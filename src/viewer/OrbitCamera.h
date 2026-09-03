#pragma once

#include "core/math/Scalar.h"
#include "core/math/Vector3.h"

namespace microsw::viewer
{

// Mathematical orbit camera: +Z world up, yaw/pitch in radians.
// Only target, distance, yaw and pitch are stored; all vectors are derived.
class OrbitCamera
{
public:
    // Origin target, distance 10, yaw -pi/4, pitch pi/6.
    OrbitCamera();
    OrbitCamera(
        const math::Vector3& target,
        math::Scalar distance,
        math::Scalar yaw,
        math::Scalar pitch);

    [[nodiscard]] const math::Vector3& target() const noexcept { return target_; }
    [[nodiscard]] math::Scalar distance() const noexcept { return distance_; }
    [[nodiscard]] math::Scalar yaw() const noexcept { return yaw_; }
    [[nodiscard]] math::Scalar pitch() const noexcept { return pitch_; }

    // Throws std::overflow_error if the derived position cannot remain finite.
    [[nodiscard]] math::Vector3 position() const;
    [[nodiscard]] math::Vector3 forward() const;
    [[nodiscard]] math::Vector3 right() const;
    [[nodiscard]] math::Vector3 up() const;

    // Non-finite inputs throw std::invalid_argument without changing state.
    void setTarget(const math::Vector3& target);
    // Any finite distance > 0 is accepted; no near-zero comparison threshold.
    void setDistance(math::Scalar distance);
    // Finite yaw is stored without wrapping.
    void setYaw(math::Scalar yaw);
    // Finite pitch is clamped to +/- (pi/2 - 1e-4 radians).
    void setPitch(math::Scalar pitch);

private:
    [[nodiscard]] math::Vector3 radialDirection() const;

    math::Vector3 target_{};
    math::Scalar distance_{};
    math::Scalar yaw_{};
    math::Scalar pitch_{};
};

}
