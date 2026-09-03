#include "core/math/Transformations.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>

#include <array>
#include <numbers>
#include <stdexcept>

namespace
{

using microsw::math::Matrix4;
using microsw::math::Scalar;
using microsw::math::Vector3;
using microsw::math::almostEqual;
using microsw::math::defaultAbsoluteTolerance;
using microsw::math::rotationX;
using microsw::math::rotationY;
using microsw::math::rotationZ;
using microsw::math::scaling;
using microsw::math::transformDirection;
using microsw::math::transformPoint;
using microsw::math::translation;

constexpr Scalar quarterTurn = std::numbers::pi_v<Scalar> / Scalar{2.0};
constexpr Vector3 xAxis{1.0, 0.0, 0.0};
constexpr Vector3 yAxis{0.0, 1.0, 0.0};
constexpr Vector3 zAxis{0.0, 0.0, 1.0};

TEST(Transformations, TranslationHasExpectedElements)
{
    const Matrix4 expected{
        1.0, 0.0, 0.0, 2.0,
        0.0, 1.0, 0.0, -3.0,
        0.0, 0.0, 1.0, 4.0,
        0.0, 0.0, 0.0, 1.0};
    EXPECT_TRUE(almostEqual(translation(Vector3{2.0, -3.0, 4.0}), expected));
}

TEST(Transformations, TranslationAffectsPoint)
{
    const Vector3 point{1.0, 2.0, 3.0};
    const Vector3 offset{2.0, -3.0, 4.0};
    EXPECT_TRUE(almostEqual(transformPoint(translation(offset), point), point + offset));
}

TEST(Transformations, TranslationDoesNotAffectDirection)
{
    const Vector3 direction{1.0, 2.0, 3.0};
    EXPECT_TRUE(almostEqual(
        transformDirection(translation(Vector3{2.0, -3.0, 4.0}), direction), direction));
}

TEST(Transformations, ScalingHasExpectedElements)
{
    const Matrix4 expected{
        2.0, 0.0, 0.0, 0.0,
        0.0, -3.0, 0.0, 0.0,
        0.0, 0.0, 4.0, 0.0,
        0.0, 0.0, 0.0, 1.0};
    EXPECT_TRUE(almostEqual(scaling(Vector3{2.0, -3.0, 4.0}), expected));
}

TEST(Transformations, ScalingAffectsPoint)
{
    EXPECT_TRUE(almostEqual(
        transformPoint(scaling(Vector3{2.0, -3.0, 4.0}), Vector3{1.0, 2.0, 3.0}),
        Vector3{2.0, -6.0, 12.0}));
}

TEST(Transformations, ScalingAffectsDirection)
{
    EXPECT_TRUE(almostEqual(
        transformDirection(scaling(Vector3{2.0, -3.0, 4.0}), Vector3{1.0, 2.0, 3.0}),
        Vector3{2.0, -6.0, 12.0}));
}

TEST(Transformations, ZeroScaleIsAllowed)
{
    const Matrix4 matrix = scaling(Vector3{0.0, 0.0, 0.0});
    EXPECT_TRUE(almostEqual(transformPoint(matrix, Vector3{1.0, 2.0, 3.0}), Vector3{}));
    EXPECT_TRUE(almostEqual(transformDirection(matrix, Vector3{1.0, 2.0, 3.0}), Vector3{}));
}

TEST(Transformations, RotationXIsIdentityAtZero)
{
    EXPECT_TRUE(almostEqual(rotationX(Scalar{0.0}), Matrix4::identity()));
}

TEST(Transformations, RotationYIsIdentityAtZero)
{
    EXPECT_TRUE(almostEqual(rotationY(Scalar{0.0}), Matrix4::identity()));
}

TEST(Transformations, RotationZIsIdentityAtZero)
{
    EXPECT_TRUE(almostEqual(rotationZ(Scalar{0.0}), Matrix4::identity()));
}

TEST(Transformations, PositiveXRotationIsRightHanded)
{
    const Matrix4 matrix = rotationX(quarterTurn);
    EXPECT_TRUE(almostEqual(transformDirection(matrix, yAxis), zAxis));
    EXPECT_TRUE(almostEqual(transformDirection(matrix, zAxis), -yAxis));
    EXPECT_TRUE(almostEqual(transformPoint(matrix, xAxis), xAxis));
}

TEST(Transformations, PositiveYRotationIsRightHanded)
{
    const Matrix4 matrix = rotationY(quarterTurn);
    EXPECT_TRUE(almostEqual(transformDirection(matrix, zAxis), xAxis));
    EXPECT_TRUE(almostEqual(transformDirection(matrix, xAxis), -zAxis));
    EXPECT_TRUE(almostEqual(transformPoint(matrix, yAxis), yAxis));
}

TEST(Transformations, PositiveZRotationIsRightHanded)
{
    const Matrix4 matrix = rotationZ(quarterTurn);
    EXPECT_TRUE(almostEqual(transformDirection(matrix, xAxis), yAxis));
    EXPECT_TRUE(almostEqual(transformDirection(matrix, yAxis), -xAxis));
    EXPECT_TRUE(almostEqual(transformPoint(matrix, zAxis), zAxis));
}

TEST(Transformations, NegativeZRotationMapsXToNegativeY)
{
    EXPECT_TRUE(almostEqual(transformDirection(rotationZ(-quarterTurn), xAxis), -yAxis));
    EXPECT_TRUE(almostEqual(transformPoint(rotationZ(-quarterTurn), xAxis), -yAxis));
}

TEST(Transformations, IdentityPreservesPoint)
{
    const Vector3 point{2.0, -3.0, 4.0};
    EXPECT_TRUE(almostEqual(transformPoint(Matrix4::identity(), point), point));
}

TEST(Transformations, IdentityPreservesDirection)
{
    const Vector3 direction{2.0, -3.0, 4.0};
    EXPECT_TRUE(almostEqual(transformDirection(Matrix4::identity(), direction), direction));
}

TEST(Transformations, PointCompositionAppliesScaleThenRotationThenTranslation)
{
    const Matrix4 scale = scaling(Vector3{2.0, 2.0, 2.0});
    const Matrix4 rotation = rotationZ(quarterTurn);
    const Matrix4 shift = translation(Vector3{10.0, 0.0, 0.0});
    const Vector3 point{1.0, 0.0, 0.0};

    const Vector3 scaled = transformPoint(scale, point);
    const Vector3 rotated = transformPoint(rotation, scaled);
    const Vector3 translated = transformPoint(shift, rotated);
    EXPECT_TRUE(almostEqual(scaled, Vector3{2.0, 0.0, 0.0}));
    EXPECT_TRUE(almostEqual(rotated, Vector3{0.0, 2.0, 0.0}));
    EXPECT_TRUE(almostEqual(translated, Vector3{10.0, 2.0, 0.0}));
    EXPECT_TRUE(almostEqual(
        transformPoint(shift * rotation * scale, point), Vector3{10.0, 2.0, 0.0}));
}

TEST(Transformations, DirectionCompositionExcludesTranslation)
{
    const Matrix4 matrix = translation(Vector3{10.0, 0.0, 0.0})
        * rotationZ(quarterTurn) * scaling(Vector3{2.0, 2.0, 2.0});
    EXPECT_TRUE(almostEqual(transformDirection(matrix, xAxis), Vector3{0.0, 2.0, 0.0}));
}

TEST(Transformations, AllGeneratedTransformsHaveAffineLastRow)
{
    const std::array<Matrix4, 5> matrices{
        translation(Vector3{2.0, -3.0, 4.0}),
        scaling(Vector3{0.0, -2.0, 3.0}),
        rotationX(Scalar{0.7}),
        rotationY(Scalar{-0.4}),
        rotationZ(Scalar{1.2})};

    for (const Matrix4& matrix : matrices)
    {
        EXPECT_TRUE(almostEqual(matrix(3, 0), Scalar{0.0}));
        EXPECT_TRUE(almostEqual(matrix(3, 1), Scalar{0.0}));
        EXPECT_TRUE(almostEqual(matrix(3, 2), Scalar{0.0}));
        EXPECT_TRUE(almostEqual(matrix(3, 3), Scalar{1.0}));
    }
}

TEST(Transformations, PointRejectsProjectiveResultInsteadOfDividing)
{
    const Matrix4 matrix{
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 2.0};
    EXPECT_THROW(
        static_cast<void>(transformPoint(matrix, Vector3{2.0, 4.0, 6.0})),
        std::domain_error);
}

TEST(Transformations, PointChecksAllContributionsToHomogeneousW)
{
    const Matrix4 matrix{
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        1.0, 2.0, 3.0, 1.0};
    for (const Vector3& point : {xAxis, yAxis, zAxis})
    {
        EXPECT_THROW(static_cast<void>(transformPoint(matrix, point)), std::domain_error);
    }
}

TEST(Transformations, DirectionRejectsProjectiveResult)
{
    const Matrix4 matrix{
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        1.0, -2.0, 3.0, 1.0};
    for (const Vector3& direction : {xAxis, yAxis, zAxis})
    {
        EXPECT_THROW(static_cast<void>(transformDirection(matrix, direction)), std::domain_error);
    }
}

TEST(Transformations, PointWValidationUsesToleranceWithoutDivision)
{
    const Matrix4 matrix{
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0 + defaultAbsoluteTolerance * Scalar{0.5}};
    const Vector3 result = transformPoint(matrix, Vector3{2.0, 3.0, 4.0});
    EXPECT_DOUBLE_EQ(result.x(), Scalar{2.0});
    EXPECT_DOUBLE_EQ(result.y(), Scalar{3.0});
    EXPECT_DOUBLE_EQ(result.z(), Scalar{4.0});
}

TEST(Transformations, DirectionWValidationUsesTolerance)
{
    const Matrix4 matrix{
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        defaultAbsoluteTolerance * Scalar{0.5}, 0.0, 0.0, 1.0};
    EXPECT_TRUE(almostEqual(transformDirection(matrix, xAxis), xAxis));
    EXPECT_THROW(
        static_cast<void>(transformDirection(matrix, Vector3{4.0, 0.0, 0.0})),
        std::domain_error);
}

}
