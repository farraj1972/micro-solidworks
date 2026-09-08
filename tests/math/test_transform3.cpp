#include "core/math/Transform3.h"

#include "core/math/Transformations.h"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace microsw::math {
namespace {

constexpr Scalar kTolerance = 1e-12;

void expectMatrixNear(const Matrix4& actual, const Matrix4& expected) {
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            EXPECT_NEAR(actual(row, column), expected(row, column), kTolerance);
        }
    }
}

TEST(Transform3, DefaultsToIdentityTrs) {
    const Transform3 transform;

    EXPECT_TRUE(almostEqual(transform.translation(), Vector3(0.0, 0.0, 0.0)));
    EXPECT_TRUE(almostEqual(transform.rotation(), Vector3(0.0, 0.0, 0.0)));
    EXPECT_TRUE(almostEqual(transform.scale(), Vector3(1.0, 1.0, 1.0)));
    EXPECT_TRUE(almostEqual(transform.matrix(), Matrix4::identity()));
}

TEST(Transform3, ConstructorAndSettersExposeAuthoritativeTrs) {
    Transform3 transform(
        Vector3(1.0, 2.0, 3.0), Vector3(0.1, 0.2, 0.3), Vector3(2.0, 3.0, 4.0));

    EXPECT_TRUE(almostEqual(transform.translation(), Vector3(1.0, 2.0, 3.0)));
    EXPECT_TRUE(almostEqual(transform.rotation(), Vector3(0.1, 0.2, 0.3)));
    EXPECT_TRUE(almostEqual(transform.scale(), Vector3(2.0, 3.0, 4.0)));

    transform.setTranslation(Vector3(-4.0, 5.0, -6.0));
    transform.setRotation(Vector3(-0.4, 0.5, -0.6));
    transform.setScale(Vector3(0.5, 1.5, 2.5));

    EXPECT_TRUE(almostEqual(transform.translation(), Vector3(-4.0, 5.0, -6.0)));
    EXPECT_TRUE(almostEqual(transform.rotation(), Vector3(-0.4, 0.5, -0.6)));
    EXPECT_TRUE(almostEqual(transform.scale(), Vector3(0.5, 1.5, 2.5)));
}

TEST(Transform3, RejectsNonFiniteTranslationAndRotation) {
    const std::array<Scalar, 3> invalidValues{
        std::numeric_limits<Scalar>::quiet_NaN(),
        std::numeric_limits<Scalar>::infinity(),
        -std::numeric_limits<Scalar>::infinity(),
    };

    for (const Scalar invalid : invalidValues) {
        Transform3 transform;
        EXPECT_THROW(transform.setTranslation(Vector3(invalid, 0.0, 0.0)), std::invalid_argument);
        EXPECT_THROW(transform.setTranslation(Vector3(0.0, invalid, 0.0)), std::invalid_argument);
        EXPECT_THROW(transform.setTranslation(Vector3(0.0, 0.0, invalid)), std::invalid_argument);
        EXPECT_THROW(transform.setRotation(Vector3(invalid, 0.0, 0.0)), std::invalid_argument);
        EXPECT_THROW(transform.setRotation(Vector3(0.0, invalid, 0.0)), std::invalid_argument);
        EXPECT_THROW(transform.setRotation(Vector3(0.0, 0.0, invalid)), std::invalid_argument);
    }
}

TEST(Transform3, RejectsNonFiniteZeroAndNegativeScaleComponents) {
    const std::array<Scalar, 5> invalidValues{
        0.0,
        -1.0,
        std::numeric_limits<Scalar>::quiet_NaN(),
        std::numeric_limits<Scalar>::infinity(),
        -std::numeric_limits<Scalar>::infinity(),
    };

    for (const Scalar invalid : invalidValues) {
        Transform3 transform;
        EXPECT_THROW(transform.setScale(Vector3(invalid, 1.0, 1.0)), std::invalid_argument);
        EXPECT_THROW(transform.setScale(Vector3(1.0, invalid, 1.0)), std::invalid_argument);
        EXPECT_THROW(transform.setScale(Vector3(1.0, 1.0, invalid)), std::invalid_argument);
    }
}

TEST(Transform3, InvalidSettersPreserveExistingState) {
    Transform3 transform(
        Vector3(1.0, 2.0, 3.0), Vector3(0.1, 0.2, 0.3), Vector3(2.0, 3.0, 4.0));
    const Matrix4 originalMatrix = transform.matrix();
    const Scalar infinity = std::numeric_limits<Scalar>::infinity();

    EXPECT_THROW(transform.setTranslation(Vector3(4.0, infinity, 6.0)), std::invalid_argument);
    EXPECT_THROW(transform.setRotation(Vector3(0.4, 0.5, infinity)), std::invalid_argument);
    EXPECT_THROW(transform.setScale(Vector3(5.0, 0.0, 7.0)), std::invalid_argument);

    EXPECT_TRUE(almostEqual(transform.translation(), Vector3(1.0, 2.0, 3.0)));
    EXPECT_TRUE(almostEqual(transform.rotation(), Vector3(0.1, 0.2, 0.3)));
    EXPECT_TRUE(almostEqual(transform.scale(), Vector3(2.0, 3.0, 4.0)));
    EXPECT_TRUE(almostEqual(transform.matrix(), originalMatrix));
}

TEST(Transform3, ProducesTranslationMatrix) {
    Transform3 transform;
    transform.setTranslation(Vector3(3.0, -4.0, 5.0));

    EXPECT_TRUE(
        almostEqual(transform.matrix(), microsw::math::translation(Vector3(3.0, -4.0, 5.0))));
}

TEST(Transform3, ProducesRotationMatricesForEachEulerAxis) {
    constexpr Scalar quarterTurn = std::numbers::pi_v<Scalar> / 2.0;
    Transform3 transform;

    transform.setRotation(Vector3(quarterTurn, 0.0, 0.0));
    expectMatrixNear(transform.matrix(), rotationX(quarterTurn));
    transform.setRotation(Vector3(0.0, quarterTurn, 0.0));
    expectMatrixNear(transform.matrix(), rotationY(quarterTurn));
    transform.setRotation(Vector3(0.0, 0.0, quarterTurn));
    expectMatrixNear(transform.matrix(), rotationZ(quarterTurn));
}

TEST(Transform3, ComposesEulerRotationAsRzRyRx) {
    const Vector3 angles(0.31, -0.47, 0.83);
    Transform3 transform;
    transform.setRotation(angles);

    const Matrix4 expected = rotationZ(angles.z()) * rotationY(angles.y())
                           * rotationX(angles.x());
    expectMatrixNear(transform.matrix(), expected);
}

TEST(Transform3, ProducesPerAxisScaleMatrix) {
    Transform3 transform;
    transform.setScale(Vector3(2.0, 3.0, 4.0));

    EXPECT_TRUE(almostEqual(transform.matrix(), scaling(Vector3(2.0, 3.0, 4.0))));
}

TEST(Transform3, ComposesCompleteTransformAsTranslationRotationScale) {
    const Vector3 translationValue(3.0, -4.0, 5.0);
    const Vector3 rotationValue(0.31, -0.47, 0.83);
    const Vector3 scaleValue(2.0, 3.0, 4.0);
    const Transform3 transform(translationValue, rotationValue, scaleValue);

    const Matrix4 expected = microsw::math::translation(translationValue)
                           * rotationZ(rotationValue.z())
                           * rotationY(rotationValue.y())
                           * rotationX(rotationValue.x())
                           * scaling(scaleValue);
    expectMatrixNear(transform.matrix(), expected);
}

TEST(Transform3, AppliesTranslationToPointsButNotDirections) {
    Transform3 transform;
    transform.setTranslation(Vector3(10.0, 20.0, 30.0));
    transform.setScale(Vector3(2.0, 3.0, 4.0));

    EXPECT_TRUE(almostEqual(transformPoint(transform.matrix(), Vector3(1.0, 1.0, 1.0)),
                            Vector3(12.0, 23.0, 34.0)));
    EXPECT_TRUE(almostEqual(transformDirection(transform.matrix(), Vector3(1.0, 1.0, 1.0)),
                            Vector3(2.0, 3.0, 4.0)));
}

} // namespace
} // namespace microsw::math
