#include "viewer/OrbitCamera.h"

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
    setYaw(yaw);
    setPitch(pitch);
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
    yaw_ = yaw;
}

void OrbitCamera::setPitch(Scalar pitch)
{
    validateFinite(pitch, "camera pitch must be finite");
    pitch_ = std::clamp(pitch, -maxPitch, maxPitch);
}

Vector3 OrbitCamera::radialDirection() const
{
    const Scalar horizontal = std::cos(pitch_);
    return {
        horizontal * std::cos(yaw_),
        horizontal * std::sin(yaw_),
        std::sin(pitch_)};
}

Vector3 OrbitCamera::position() const
{
    const Vector3 result = target_ + radialDirection() * distance_;
    if (!isFinite(result))
    {
        throw std::overflow_error{"derived camera position is not finite"};
    }
    return result;
}

Vector3 OrbitCamera::forward() const
{
    // Equivalent to normalized(target - position), without subtracting large
    // positions or normalizing a distance smaller than Math's zero tolerance.
    return -radialDirection().normalized();
}

Vector3 OrbitCamera::right() const
{
    return math::cross(forward(), worldUp).normalized();
}

Vector3 OrbitCamera::up() const
{
    // (right, up, -forward) is right-handed: cross(right, up) = -forward.
    return math::cross(right(), forward());
}

}
