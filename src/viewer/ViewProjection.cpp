#include "viewer/ViewProjection.h"

#include "viewer/OrbitCamera.h"

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace microsw::viewer
{
namespace
{

using math::Matrix4;
using math::Scalar;

void validateProjection(Scalar size, Scalar aspect, Scalar nearPlane, Scalar farPlane)
{
    if (!std::isfinite(size) || !std::isfinite(aspect)
        || !std::isfinite(nearPlane) || !std::isfinite(farPlane)
        || size <= 0.0 || aspect <= 0.0 || nearPlane <= 0.0 || farPlane <= nearPlane)
    {
        throw std::invalid_argument{"invalid projection parameters"};
    }
}

Matrix4 checkedMatrix(const Matrix4& matrix)
{
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
        {
            if (!std::isfinite(matrix(row, column)))
            {
                throw std::overflow_error{"view/projection matrix coefficient is not finite"};
            }
        }
    }
    return matrix;
}

}

math::Matrix4 viewMatrix(const OrbitCamera& camera)
{
    const auto eye = camera.position();
    const auto right = camera.right();
    const auto up = camera.up();
    const auto forward = camera.forward();

    // Basis vectors are rows: dot products give camera-space coordinates.
    return checkedMatrix({
        right.x(), right.y(), right.z(), -math::dot(right, eye),
        up.x(), up.y(), up.z(), -math::dot(up, eye),
        -forward.x(), -forward.y(), -forward.z(), math::dot(forward, eye),
        0.0, 0.0, 0.0, 1.0});
}

math::Matrix4 perspective(
    Scalar verticalFovRadians, Scalar aspectRatio, Scalar nearPlane, Scalar farPlane)
{
    validateProjection(verticalFovRadians, aspectRatio, nearPlane, farPlane);
    if (verticalFovRadians >= std::numbers::pi_v<Scalar>)
    {
        throw std::invalid_argument{"vertical FOV must be less than pi radians"};
    }

    const Scalar f = 1.0 / std::tan(verticalFovRadians / 2.0);
    const Scalar depth = farPlane - nearPlane;
    // Algebraically -(far+near)/depth and -2*far*near/depth.
    // Divide first to avoid unnecessary overflow in sums/products.
    const Scalar zScale = -(farPlane / depth + nearPlane / depth);
    const Scalar zOffset = -2.0 * (nearPlane * (farPlane / depth));
    return checkedMatrix({
        f / aspectRatio, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, zScale, zOffset,
        0.0, 0.0, -1.0, 0.0});
}

math::Matrix4 orthographic(
    Scalar visibleHeight, Scalar aspectRatio, Scalar nearPlane, Scalar farPlane)
{
    validateProjection(visibleHeight, aspectRatio, nearPlane, farPlane);
    const Scalar depth = farPlane - nearPlane;
    // 2 / visibleWidth, evaluated without forming visibleHeight * aspect.
    const Scalar xScale = (2.0 / visibleHeight) / aspectRatio;
    return checkedMatrix({
        xScale, 0.0, 0.0, 0.0,
        0.0, 2.0 / visibleHeight, 0.0, 0.0,
        0.0, 0.0, -2.0 / depth, -(farPlane / depth + nearPlane / depth),
        0.0, 0.0, 0.0, 1.0});
}

}
