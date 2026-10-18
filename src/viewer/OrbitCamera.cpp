#include "viewer/OrbitCamera.h"

#include "core/math/Tolerance.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace microsw::viewer
{
namespace
{

using math::Scalar;
using math::Vector3;

// Navigation singularity guard, not an equality-comparison tolerance.
constexpr Scalar poleMargin = 1.0e-4;
constexpr Scalar maxPitch = std::numbers::pi_v<Scalar> / Scalar{2.0} - poleMargin;
constexpr Vector3 worldUp{0.0, 0.0, 1.0};

bool isFinite(const Vector3& vector)
{
    return std::isfinite(vector.x())
        && std::isfinite(vector.y())
        && std::isfinite(vector.z());
}

Vector3 rotate(const Vector3& value, const Vector3& axis, Scalar angle)
{
    const auto unit = axis.normalized();
    return value * std::cos(angle)
        + math::cross(unit, value) * std::sin(angle)
        + unit * math::dot(unit, value) * (Scalar{1} - std::cos(angle));
}

void validateFinite(Scalar value, const char* message)
{
    if (!std::isfinite(value))
    {
        throw std::invalid_argument{message};
    }
}

}

OrbitCamera::OrbitCamera()
    : OrbitCamera(
          Vector3{}, Scalar{10.0},
          -std::numbers::pi_v<Scalar> / Scalar{4.0},
          std::numbers::pi_v<Scalar> / Scalar{6.0})
{
}

OrbitCamera::OrbitCamera(
    const Vector3& target, Scalar distance, Scalar yaw, Scalar pitch)
{
    setTarget(target);
    setDistance(distance);
    setAngles(yaw, pitch);
}

void OrbitCamera::setTarget(const Vector3& target)
{
    if (!isFinite(target))
    {
        throw std::invalid_argument{"camera target components must be finite"};
    }
    target_ = target;
}

void OrbitCamera::setDistance(Scalar distance)
{
    validateFinite(distance, "camera distance must be finite");
    if (distance <= Scalar{0.0})
    {
        throw std::invalid_argument{"camera distance must be positive"};
    }
    distance_ = distance;
}

void OrbitCamera::setYaw(Scalar yaw)
{
    validateFinite(yaw, "camera yaw must be finite");
    setAngles(yaw, pitch());
}

void OrbitCamera::setPitch(Scalar pitch)
{
    validateFinite(pitch, "camera pitch must be finite");
    setAngles(yaw(), pitch);
}

void OrbitCamera::setAngles(Scalar yaw, Scalar pitch)
{
    validateFinite(yaw, "camera yaw must be finite");
    validateFinite(pitch, "camera pitch must be finite");
    pitch = std::clamp(pitch, -maxPitch, maxPitch);
    const Scalar horizontal = std::cos(pitch);
    const Vector3 radial{
        horizontal * std::cos(yaw),
        horizontal * std::sin(yaw),
        std::sin(pitch)};
    const auto forward = -radial;
    const auto right = math::cross(forward, worldUp).normalized();
    setOrientation(forward, math::cross(right, forward).normalized());
}

Scalar OrbitCamera::yaw() const noexcept
{
    const auto radial = -forward_;
    return std::atan2(radial.y(), radial.x());
}

Scalar OrbitCamera::pitch() const noexcept
{
    return std::asin(std::clamp((-forward_).z(), Scalar{-1}, Scalar{1}));
}

Vector3 OrbitCamera::position() const
{
    const Vector3 result = target_ - forward_ * distance_;
    if (!isFinite(result))
    {
        throw std::overflow_error{"derived camera position is not finite"};
    }
    return result;
}

Vector3 OrbitCamera::forward() const
{
    return forward_;
}

Vector3 OrbitCamera::right() const
{
    return math::cross(forward_, up_).normalized();
}

Vector3 OrbitCamera::up() const
{
    return up_;
}

void OrbitCamera::setOrientation(const Vector3& forward, const Vector3& up)
{
    if (!isFinite(forward) || !isFinite(up))
        throw std::invalid_argument{"camera orientation components must be finite"};
    const auto f = forward.normalized();
    const auto suppliedUp = up.normalized();
    if (std::abs(math::dot(f, suppliedUp)) > math::defaultAbsoluteTolerance)
        throw std::invalid_argument{"camera forward and up must be perpendicular"};
    const auto r = math::cross(f, suppliedUp);
    if (r.length() <= math::defaultAbsoluteTolerance)
        throw std::invalid_argument{"camera forward and up must not be parallel"};
    const auto right = r.normalized();
    forward_ = f;
    up_ = math::cross(right, f).normalized();
}

void OrbitCamera::orbit(Scalar yawDelta, Scalar pitchDelta)
{
    validateFinite(yawDelta, "camera orbit yaw delta must be finite");
    validateFinite(pitchDelta, "camera orbit pitch delta must be finite");
    auto nextForward = rotate(forward_, up_, yawDelta).normalized();
    auto nextRight = math::cross(nextForward, up_).normalized();
    const auto nextUp = rotate(up_, nextRight, -pitchDelta).normalized();
    nextForward = rotate(nextForward, nextRight, -pitchDelta).normalized();
    setOrientation(nextForward, nextUp);
}

}
