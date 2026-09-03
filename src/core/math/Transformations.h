#pragma once

#include "core/math/Matrix4.h"
#include "core/math/Scalar.h"
#include "core/math/Vector3.h"

namespace microsw::math
{

[[nodiscard]] Matrix4 translation(const Vector3& offset);
[[nodiscard]] Matrix4 scaling(const Vector3& factors);

// Right-handed rotations; all angles are in radians.
[[nodiscard]] Matrix4 rotationX(Scalar radians);
[[nodiscard]] Matrix4 rotationY(Scalar radians);
[[nodiscard]] Matrix4 rotationZ(Scalar radians);

// Affine operation on (x,y,z,1): requires resulting w approximately 1.
// Throws std::domain_error otherwise; never performs perspective division.
[[nodiscard]] Vector3 transformPoint(const Matrix4& matrix, const Vector3& point);

// Affine operation on (x,y,z,0): requires resulting w approximately 0.
// Translation is excluded. Throws std::domain_error if the w check fails.
[[nodiscard]] Vector3 transformDirection(const Matrix4& matrix, const Vector3& direction);

}
