#include "core/math/Transformations.h"

#include "core/math/Tolerance.h"

#include <cmath>
#include <stdexcept>

namespace microsw::math
{

Matrix4 translation(const Vector3& offset)
{
    return {
        1.0, 0.0, 0.0, offset.x(),
        0.0, 1.0, 0.0, offset.y(),
        0.0, 0.0, 1.0, offset.z(),
        0.0, 0.0, 0.0, 1.0};
}

Matrix4 scaling(const Vector3& factors)
{
    return {
        factors.x(), 0.0, 0.0, 0.0,
        0.0, factors.y(), 0.0, 0.0,
        0.0, 0.0, factors.z(), 0.0,
        0.0, 0.0, 0.0, 1.0};
}

Matrix4 rotationX(Scalar radians)
{
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);

    return {
        1.0, 0.0, 0.0, 0.0,
        0.0, cosine, -sine, 0.0,
        0.0, sine, cosine, 0.0,
        0.0, 0.0, 0.0, 1.0};
}

Matrix4 rotationY(Scalar radians)
{
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);

    return {
        cosine, 0.0, sine, 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sine, 0.0, cosine, 0.0,
        0.0, 0.0, 0.0, 1.0};
}

Matrix4 rotationZ(Scalar radians)
{
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);

    return {
        cosine, -sine, 0.0, 0.0,
        sine, cosine, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0};
}

Vector3 transformPoint(const Matrix4& matrix, const Vector3& point)
{
    const Scalar w = matrix(3, 0) * point.x()
        + matrix(3, 1) * point.y()
        + matrix(3, 2) * point.z()
        + matrix(3, 3);
    if (!almostEqual(w, Scalar{1.0}))
    {
        throw std::domain_error{"transformPoint requires resulting w approximately 1"};
    }

    return {
        matrix(0, 0) * point.x() + matrix(0, 1) * point.y()
            + matrix(0, 2) * point.z() + matrix(0, 3),
        matrix(1, 0) * point.x() + matrix(1, 1) * point.y()
            + matrix(1, 2) * point.z() + matrix(1, 3),
        matrix(2, 0) * point.x() + matrix(2, 1) * point.y()
            + matrix(2, 2) * point.z() + matrix(2, 3)};
}

Vector3 transformDirection(const Matrix4& matrix, const Vector3& direction)
{
    const Scalar w = matrix(3, 0) * direction.x()
        + matrix(3, 1) * direction.y()
        + matrix(3, 2) * direction.z();
    if (!isNearlyZero(w))
    {
        throw std::domain_error{"transformDirection requires resulting w approximately 0"};
    }

    return {
        matrix(0, 0) * direction.x() + matrix(0, 1) * direction.y()
            + matrix(0, 2) * direction.z(),
        matrix(1, 0) * direction.x() + matrix(1, 1) * direction.y()
            + matrix(1, 2) * direction.z(),
        matrix(2, 0) * direction.x() + matrix(2, 1) * direction.y()
            + matrix(2, 2) * direction.z()};
}

}
