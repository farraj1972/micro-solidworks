#include "core/math/Matrix3.h"
#include "core/math/Matrix4.h"
#include "core/math/Tolerance.h"
#include "core/math/Transformations.h"
#include "core/math/Vector2.h"
#include "core/math/Vector3.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <numbers>

namespace
{

using microsw::math::Matrix3;
using microsw::math::Matrix4;
using microsw::math::Scalar;
using microsw::math::Vector2;
using microsw::math::Vector3;
using microsw::math::almostEqual;
using microsw::math::cross;
using microsw::math::dot;
using microsw::math::rotationX;
using microsw::math::rotationY;
using microsw::math::rotationZ;
using microsw::math::scaling;
using microsw::math::transformDirection;
using microsw::math::transformPoint;
using microsw::math::translation;

Matrix4 composedRotation()
{
    return rotationZ(Scalar{0.63}) * rotationY(Scalar{-0.41}) * rotationX(Scalar{0.37});
}

TEST(MathIntegration, RotationPreservesVectorLength)
{
    const Vector3 vector{2.0, -3.0, 4.0};
    const std::array<Matrix4, 3> rotations{
        rotationX(Scalar{0.37}), rotationY(Scalar{-0.41}), rotationZ(Scalar{0.63})};

    for (std::size_t axis = 0; axis < rotations.size(); ++axis)
    {
        SCOPED_TRACE(axis);
        EXPECT_TRUE(almostEqual(
            transformDirection(rotations[axis], vector).length(), vector.length()));
    }
}

TEST(MathIntegration, ComposedRotationPreservesDotProduct)
{
    const Vector3 first{2.0, -3.0, 4.0};
    const Vector3 second{-1.5, 0.7, 2.3};
    const Matrix4 rotation = composedRotation();

    EXPECT_TRUE(almostEqual(
        dot(transformDirection(rotation, first), transformDirection(rotation, second)),
        dot(first, second)));
}

TEST(MathIntegration, ComposedRotationPreservesCrossOrientation)
{
    const Vector3 first{2.0, -3.0, 4.0};
    const Vector3 second{-1.5, 0.7, 2.3};
    const Matrix4 rotation = composedRotation();

    EXPECT_TRUE(almostEqual(
        transformDirection(rotation, cross(first, second)),
        cross(transformDirection(rotation, first), transformDirection(rotation, second))));
}

TEST(MathIntegration, RotatedBasisRemainsOrthonormalAndRightHanded)
{
    const Matrix4 rotation = composedRotation();
    const Vector3 x = transformDirection(rotation, Vector3{1.0, 0.0, 0.0});
    const Vector3 y = transformDirection(rotation, Vector3{0.0, 1.0, 0.0});
    const Vector3 z = transformDirection(rotation, Vector3{0.0, 0.0, 1.0});

    EXPECT_TRUE(almostEqual(x.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(y.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(z.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(dot(x, y), Scalar{0.0}));
    EXPECT_TRUE(almostEqual(dot(y, z), Scalar{0.0}));
    EXPECT_TRUE(almostEqual(dot(z, x), Scalar{0.0}));
    EXPECT_TRUE(almostEqual(cross(x, y), z));
}

TEST(MathIntegration, RotationTransposeActsAsTwoSidedInverse)
{
    const Matrix4 rotation = composedRotation();

    // This property applies to pure rotations, not arbitrary affine matrices.
    EXPECT_TRUE(almostEqual(rotation.transposed() * rotation, Matrix4::identity()));
    EXPECT_TRUE(almostEqual(rotation * rotation.transposed(), Matrix4::identity()));
}

TEST(MathIntegration, ComposedPointTransformMatchesSequentialApplication)
{
    const Vector3 point{1.2, -2.3, 0.7};
    const Matrix4 scale = scaling(Vector3{2.0, 0.5, -1.5});
    const Matrix4 rotation = rotationZ(Scalar{0.63}) * rotationY(Scalar{-0.41});
    const Matrix4 shift = translation(Vector3{10.0, -4.0, 3.0});
    const Vector3 sequential =
        transformPoint(shift, transformPoint(rotation, transformPoint(scale, point)));
    const Vector3 composed = transformPoint(shift * rotation * scale, point);

    EXPECT_TRUE(almostEqual(sequential, composed));
}

TEST(MathIntegration, ComposedDirectionTransformMatchesSequentialApplication)
{
    const Vector3 direction{1.2, -2.3, 0.7};
    const Matrix4 scale = scaling(Vector3{2.0, 0.5, -1.5});
    const Matrix4 rotation = rotationZ(Scalar{0.63}) * rotationY(Scalar{-0.41});
    const Matrix4 shift = translation(Vector3{10.0, -4.0, 3.0});
    const Vector3 sequential = transformDirection(
        shift, transformDirection(rotation, transformDirection(scale, direction)));
    const Vector3 composed = transformDirection(shift * rotation * scale, direction);

    EXPECT_TRUE(almostEqual(sequential, composed));
    EXPECT_TRUE(almostEqual(composed, transformDirection(rotation * scale, direction)));
}

TEST(MathIntegration, PointAndDirectionTranslationDifferenceEqualsOffset)
{
    const Vector3 vector{1.2, -2.3, 0.7};
    const Vector3 offset{10.0, -4.0, 3.0};
    const Matrix4 shift = translation(offset);

    EXPECT_TRUE(almostEqual(
        transformPoint(shift, vector) - transformDirection(shift, vector), offset));
}

TEST(MathIntegration, UniformScalingScalesDotProductBySquaredFactor)
{
    const Scalar factor = 2.5;
    const Vector3 first{2.0, -3.0, 4.0};
    const Vector3 second{-1.5, 0.7, 2.3};
    const Matrix4 scale = scaling(Vector3{factor, factor, factor});

    EXPECT_TRUE(almostEqual(
        dot(transformDirection(scale, first), transformDirection(scale, second)),
        factor * factor * dot(first, second)));
}

TEST(MathIntegration, PositiveUniformScalingScalesCrossProductBySquaredFactor)
{
    const Scalar factor = 2.5;
    const Vector3 first{2.0, -3.0, 4.0};
    const Vector3 second{-1.5, 0.7, 2.3};
    const Matrix4 scale = scaling(Vector3{factor, factor, factor});

    EXPECT_TRUE(almostEqual(
        cross(transformDirection(scale, first), transformDirection(scale, second)),
        factor * factor * cross(first, second)));
}

TEST(MathIntegration, Matrix3IdentityPreservesNonTrivialVector)
{
    const Vector3 vector{1.2, -2.3, 0.7};

    EXPECT_TRUE(almostEqual(Matrix3::identity() * vector, vector));
}

TEST(MathIntegration, Matrix3ProductApplicationIsAssociative)
{
    const Matrix3 first{
        1.0, 2.0, -1.0,
        0.0, 0.5, 3.0,
        2.0, -1.0, 1.0};
    const Matrix3 second{
        2.0, 0.0, 1.0,
        -1.0, 3.0, 0.0,
        0.5, 1.0, -2.0};
    const Vector3 vector{1.2, -2.3, 0.7};

    EXPECT_TRUE(almostEqual((first * second) * vector, first * (second * vector)));
}

TEST(MathIntegration, Matrix4AffineMultiplicationIsAssociative)
{
    const Matrix4 first = translation(Vector3{10.0, -4.0, 3.0});
    const Matrix4 second = composedRotation();
    const Matrix4 third = scaling(Vector3{2.0, 0.5, -1.5});

    EXPECT_TRUE(almostEqual((first * second) * third, first * (second * third)));
}

TEST(MathIntegration, IdentityPreservesComposedAffineTransform)
{
    const Matrix4 matrix = translation(Vector3{10.0, -4.0, 3.0})
        * composedRotation() * scaling(Vector3{2.0, 0.5, -1.5});
    const Matrix4 identity = Matrix4::identity();

    EXPECT_TRUE(almostEqual(identity * matrix, matrix));
    EXPECT_TRUE(almostEqual(matrix * identity, matrix));
}

TEST(MathIntegration, ZeroVectorRetainsPointAndDirectionSemantics)
{
    const Vector3 zero{};
    const Vector3 offset{10.0, -4.0, 3.0};
    const Matrix4 shift = translation(offset);

    EXPECT_TRUE(almostEqual(transformDirection(composedRotation(), zero), zero));
    EXPECT_TRUE(almostEqual(transformDirection(shift, zero), zero));
    EXPECT_TRUE(almostEqual(transformPoint(shift, zero), offset));
}

TEST(MathIntegration, DefaultToleranceAbsorbsTrigonometricResiduals)
{
    const Matrix4 rotation = rotationZ(std::numbers::pi_v<Scalar> / Scalar{2.0});
    const Vector3 rotated = transformDirection(rotation, Vector3{1.0, 0.0, 0.0});

    // The cosine residual is accepted by B1.1; no rotation-specific epsilon.
    EXPECT_TRUE(almostEqual(rotated.x(), Scalar{0.0}));
    EXPECT_TRUE(almostEqual(rotated, Vector3{0.0, 1.0, 0.0}));
}

TEST(MathIntegration, ExplicitMatrix3RotationHasUnitDeterminant)
{
    const Scalar radians = 0.63;
    const Scalar cosine = std::cos(radians);
    const Scalar sine = std::sin(radians);
    const Matrix3 rotation{
        cosine, -sine, 0.0,
        sine, cosine, 0.0,
        0.0, 0.0, 1.0};

    EXPECT_TRUE(almostEqual(rotation.determinant(), Scalar{1.0}));
}

TEST(MathIntegration, Vector2AndVector3AgreeOnPlanarNormalizationAndDotProduct)
{
    const Vector2 first{1.2, -2.3};
    const Vector2 second{-0.7, 3.1};
    // Explicit embedding in the XY plane is local to this test, not a new API.
    const Vector3 first3{first.x(), first.y(), 0.0};
    const Vector3 second3{second.x(), second.y(), 0.0};
    const Vector2 normalized = first.normalized();

    EXPECT_TRUE(almostEqual(first.length(), first3.length()));
    EXPECT_TRUE(almostEqual(dot(first, second), dot(first3, second3)));
    EXPECT_TRUE(almostEqual(
        Vector3{normalized.x(), normalized.y(), 0.0}, first3.normalized()));
}

}
