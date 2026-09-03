#include "viewer/OrbitCamera.h"

#include "core/math/Tolerance.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{

using microsw::math::Scalar;
using microsw::math::Vector3;
using microsw::math::almostEqual;
using microsw::math::cross;
using microsw::math::dot;
using microsw::viewer::OrbitCamera;

constexpr Scalar pi = std::numbers::pi_v<Scalar>;
const std::array<Scalar, 3> nonFinite{
    std::numeric_limits<Scalar>::quiet_NaN(),
    std::numeric_limits<Scalar>::infinity(),
    -std::numeric_limits<Scalar>::infinity()};

void expectFinite(const Vector3& vector)
{
    EXPECT_TRUE(std::isfinite(vector.x()));
    EXPECT_TRUE(std::isfinite(vector.y()));
    EXPECT_TRUE(std::isfinite(vector.z()));
}

void expectOrthonormalBasis(const OrbitCamera& camera)
{
    const Vector3 forward = camera.forward();
    const Vector3 right = camera.right();
    const Vector3 up = camera.up();
    expectFinite(forward);
    expectFinite(right);
    expectFinite(up);
    EXPECT_TRUE(almostEqual(forward.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(right.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(up.length(), Scalar{1.0}));
    EXPECT_TRUE(almostEqual(dot(forward, right), Scalar{0.0}));
    EXPECT_TRUE(almostEqual(dot(forward, up), Scalar{0.0}));
    EXPECT_TRUE(almostEqual(dot(right, up), Scalar{0.0}));
    // Forward points into the scene: the RH basis is (right, up, -forward).
    EXPECT_TRUE(almostEqual(cross(right, up), -forward));
}

TEST(OrbitCamera, DefaultCameraIsValidAndElevated)
{
    const OrbitCamera camera;
    EXPECT_TRUE(almostEqual(camera.target(), Vector3{}));
    EXPECT_TRUE(almostEqual(camera.distance(), Scalar{10.0}));
    EXPECT_TRUE(almostEqual(camera.yaw(), -pi / Scalar{4.0}));
    EXPECT_TRUE(almostEqual(camera.pitch(), pi / Scalar{6.0}));
    EXPECT_GT(camera.position().z(), Scalar{0.0});
    EXPECT_TRUE(almostEqual(camera.position().length(), camera.distance()));
    EXPECT_TRUE(almostEqual(camera.forward(), (-camera.position()).normalized()));
    expectOrthonormalBasis(camera);
}

TEST(OrbitCamera, ExplicitConstructionPreservesState)
{
    const Vector3 target{2.0, -3.0, 4.0};
    const OrbitCamera camera{target, 7.0, -0.7, 0.4};
    EXPECT_TRUE(almostEqual(camera.target(), target));
    EXPECT_TRUE(almostEqual(camera.distance(), Scalar{7.0}));
    EXPECT_TRUE(almostEqual(camera.yaw(), Scalar{-0.7}));
    EXPECT_TRUE(almostEqual(camera.pitch(), Scalar{0.4}));
}

TEST(OrbitCamera, TargetSetterTranslatesPositionWithoutChangingBasis)
{
    OrbitCamera camera{Vector3{2.0, -3.0, 4.0}, 7.0, -0.7, 0.4};
    const OrbitCamera before = camera;
    const Vector3 target{-5.0, 6.0, 1.0};
    camera.setTarget(target);
    EXPECT_TRUE(almostEqual(camera.target(), target));
    EXPECT_TRUE(almostEqual(camera.position() - before.position(), target - before.target()));
    EXPECT_TRUE(almostEqual(camera.forward(), before.forward()));
    EXPECT_TRUE(almostEqual(camera.right(), before.right()));
    EXPECT_TRUE(almostEqual(camera.up(), before.up()));
    EXPECT_TRUE(almostEqual(camera.distance(), before.distance()));
    EXPECT_TRUE(almostEqual(camera.yaw(), before.yaw()));
    EXPECT_TRUE(almostEqual(camera.pitch(), before.pitch()));
}

TEST(OrbitCamera, DistanceSetterChangesRadiusWithoutChangingOrientation)
{
    OrbitCamera camera;
    const OrbitCamera before = camera;
    camera.setDistance(23.0);
    EXPECT_TRUE(almostEqual(camera.distance(), Scalar{23.0}));
    EXPECT_TRUE(almostEqual((camera.position() - camera.target()).length(), Scalar{23.0}));
    EXPECT_TRUE(almostEqual(camera.target(), before.target()));
    EXPECT_TRUE(almostEqual(camera.forward(), before.forward()));
    EXPECT_TRUE(almostEqual(camera.right(), before.right()));
    EXPECT_TRUE(almostEqual(camera.up(), before.up()));
}

TEST(OrbitCamera, YawSetterAcceptsNegativeAndUnwrappedAngles)
{
    OrbitCamera camera;
    for (const Scalar yaw : {Scalar{-7.3}, Scalar{9.4}})
    {
        camera.setYaw(yaw);
        EXPECT_TRUE(almostEqual(camera.yaw(), yaw));
        const OrbitCamera expected{camera.target(), camera.distance(), yaw, camera.pitch()};
        EXPECT_TRUE(almostEqual(camera.position(), expected.position()));
        expectOrthonormalBasis(camera);
    }
}

TEST(OrbitCamera, ConstructorAcceptsYawBeyondFullTurn)
{
    const OrbitCamera camera{Vector3{}, 10.0, Scalar{4.0} * pi + Scalar{0.3}, 0.2};
    const OrbitCamera equivalent{Vector3{}, 10.0, 0.3, 0.2};
    EXPECT_GT(camera.yaw(), Scalar{2.0} * pi);
    EXPECT_TRUE(almostEqual(camera.position(), equivalent.position()));
    EXPECT_TRUE(almostEqual(camera.forward(), equivalent.forward()));
}

TEST(OrbitCamera, PitchSetterPreservesValuesInsideLimits)
{
    OrbitCamera camera;
    for (const Scalar pitch : {Scalar{-0.6}, Scalar{0.0}, Scalar{0.7}})
    {
        camera.setPitch(pitch);
        EXPECT_TRUE(almostEqual(camera.pitch(), pitch));
        const OrbitCamera expected{camera.target(), camera.distance(), camera.yaw(), pitch};
        EXPECT_TRUE(almostEqual(camera.position(), expected.position()));
        expectOrthonormalBasis(camera);
    }
}

TEST(OrbitCamera, ZeroYawLooksFromPositiveXWithPositiveZUp)
{
    const OrbitCamera camera{Vector3{}, 10.0, 0.0, 0.0};
    EXPECT_TRUE(almostEqual(camera.position(), Vector3{10.0, 0.0, 0.0}));
    EXPECT_TRUE(almostEqual(camera.forward(), Vector3{-1.0, 0.0, 0.0}));
    EXPECT_TRUE(almostEqual(camera.right(), Vector3{0.0, 1.0, 0.0}));
    EXPECT_TRUE(almostEqual(camera.up(), Vector3{0.0, 0.0, 1.0}));
}

TEST(OrbitCamera, QuarterTurnYawLooksFromPositiveY)
{
    const OrbitCamera camera{Vector3{}, 10.0, pi / Scalar{2.0}, 0.0};
    EXPECT_TRUE(almostEqual(camera.position(), Vector3{0.0, 10.0, 0.0}));
    EXPECT_TRUE(almostEqual(camera.forward(), Vector3{0.0, -1.0, 0.0}));
    EXPECT_TRUE(almostEqual(camera.right(), Vector3{-1.0, 0.0, 0.0}));
    EXPECT_TRUE(almostEqual(camera.up(), Vector3{0.0, 0.0, 1.0}));
}

TEST(OrbitCamera, ElevatedViewUsesZForHeight)
{
    const OrbitCamera camera{Vector3{}, 10.0, 0.0, pi / Scalar{6.0}};
    EXPECT_TRUE(almostEqual(camera.position(), Vector3{Scalar{5.0} * std::sqrt(Scalar{3.0}), 0.0, 5.0}));
    EXPECT_GT(camera.position().z(), Scalar{0.0});
    EXPECT_TRUE(almostEqual(camera.position().length(), Scalar{10.0}));
}

TEST(OrbitCamera, PositionAndForwardFollowOrbitGeometry)
{
    const Vector3 target{2.0, -3.0, 4.0};
    for (const Scalar yaw : {Scalar{-2.7}, Scalar{0.63}, Scalar{8.1}})
    {
        for (const Scalar pitch : {Scalar{-1.1}, Scalar{0.0}, Scalar{0.8}})
        {
            const OrbitCamera camera{target, 13.0, yaw, pitch};
            EXPECT_TRUE(almostEqual((camera.position() - target).length(), camera.distance()));
            EXPECT_TRUE(almostEqual(camera.forward(), (target - camera.position()).normalized()));
            expectOrthonormalBasis(camera);
        }
    }
}

TEST(OrbitCamera, UpperPitchClampKeepsBasisValid)
{
    const OrbitCamera camera{Vector3{}, 10.0, 0.63, pi};
    EXPECT_LT(camera.pitch(), pi / Scalar{2.0});
    EXPECT_TRUE(almostEqual(camera.pitch(), pi / Scalar{2.0} - Scalar{1.0e-4}));
    EXPECT_GT(camera.position().z(), Scalar{0.0});
    expectOrthonormalBasis(camera);
    OrbitCamera changed;
    changed.setPitch(pi);
    EXPECT_TRUE(almostEqual(changed.pitch(), camera.pitch()));
    expectOrthonormalBasis(changed);
}

TEST(OrbitCamera, LowerPitchClampKeepsBasisValid)
{
    const OrbitCamera camera{Vector3{}, 10.0, -0.63, -pi};
    EXPECT_GT(camera.pitch(), -pi / Scalar{2.0});
    EXPECT_TRUE(almostEqual(camera.pitch(), -pi / Scalar{2.0} + Scalar{1.0e-4}));
    EXPECT_LT(camera.position().z(), Scalar{0.0});
    expectOrthonormalBasis(camera);
    OrbitCamera changed;
    changed.setPitch(-pi);
    EXPECT_TRUE(almostEqual(changed.pitch(), camera.pitch()));
    expectOrthonormalBasis(changed);
}

TEST(OrbitCamera, NonPositiveDistanceIsRejectedWithoutMutation)
{
    for (const Scalar distance : {Scalar{0.0}, Scalar{-0.0}, Scalar{-1.0}})
    {
        EXPECT_THROW((OrbitCamera{Vector3{}, distance, 0.0, 0.0}), std::invalid_argument);
        OrbitCamera camera;
        const Scalar before = camera.distance();
        EXPECT_THROW(camera.setDistance(distance), std::invalid_argument);
        EXPECT_TRUE(almostEqual(camera.distance(), before));
    }
}

TEST(OrbitCamera, NonFiniteDistanceIsRejectedWithoutMutation)
{
    for (const Scalar distance : nonFinite)
    {
        EXPECT_THROW((OrbitCamera{Vector3{}, distance, 0.0, 0.0}), std::invalid_argument);
        OrbitCamera camera;
        EXPECT_THROW(camera.setDistance(distance), std::invalid_argument);
        EXPECT_TRUE(almostEqual(camera.distance(), Scalar{10.0}));
    }
}

TEST(OrbitCamera, NonFiniteYawIsRejectedWithoutMutation)
{
    for (const Scalar yaw : nonFinite)
    {
        EXPECT_THROW((OrbitCamera{Vector3{}, 10.0, yaw, 0.0}), std::invalid_argument);
        OrbitCamera camera;
        const Scalar before = camera.yaw();
        EXPECT_THROW(camera.setYaw(yaw), std::invalid_argument);
        EXPECT_TRUE(almostEqual(camera.yaw(), before));
    }
}

TEST(OrbitCamera, NonFinitePitchIsRejectedBeforeClampWithoutMutation)
{
    for (const Scalar pitch : nonFinite)
    {
        EXPECT_THROW((OrbitCamera{Vector3{}, 10.0, 0.0, pitch}), std::invalid_argument);
        OrbitCamera camera;
        const Scalar before = camera.pitch();
        EXPECT_THROW(camera.setPitch(pitch), std::invalid_argument);
        EXPECT_TRUE(almostEqual(camera.pitch(), before));
    }
}

TEST(OrbitCamera, EachNonFiniteTargetComponentIsRejectedWithoutMutation)
{
    for (const Scalar value : nonFinite)
    {
        for (const Vector3& target : {
                 Vector3{value, 2.0, 3.0}, Vector3{1.0, value, 3.0}, Vector3{1.0, 2.0, value}})
        {
            EXPECT_THROW((OrbitCamera{target, 10.0, 0.0, 0.0}), std::invalid_argument);
            OrbitCamera camera;
            EXPECT_THROW(camera.setTarget(target), std::invalid_argument);
            EXPECT_TRUE(almostEqual(camera.target(), Vector3{}));
        }
    }
}

TEST(OrbitCamera, PositiveDistancesBelowMathToleranceRemainValid)
{
    for (const Scalar distance : {
             Scalar{1.0e-15}, std::numeric_limits<Scalar>::min()})
    {
        OrbitCamera camera{Vector3{}, distance, 0.63, 0.4};
        EXPECT_GT(camera.distance(), Scalar{0.0});
        // Compare a ratio: default absolute tolerance alone would hide zero.
        EXPECT_TRUE(almostEqual(camera.distance() / distance, Scalar{1.0}));
        expectFinite(camera.position());
        expectOrthonormalBasis(camera);
        camera.setDistance(distance);
        expectOrthonormalBasis(camera);
    }
}

TEST(OrbitCamera, LargeTargetDoesNotDestroyDirectionThroughCancellation)
{
    const OrbitCamera origin{Vector3{}, 1.0, 0.63, 0.4};
    const OrbitCamera distant{Vector3{1.0e20, -1.0e20, 1.0e20}, 1.0, 0.63, 0.4};
    EXPECT_TRUE(almostEqual(distant.forward(), origin.forward()));
    EXPECT_TRUE(almostEqual(distant.right(), origin.right()));
    EXPECT_TRUE(almostEqual(distant.up(), origin.up()));
    expectOrthonormalBasis(distant);
}

TEST(OrbitCamera, UnrepresentablePositionFailsExplicitly)
{
    const Scalar largest = std::numeric_limits<Scalar>::max();
    const OrbitCamera camera{Vector3{largest, 0.0, 0.0}, largest, 0.0, 0.0};
    EXPECT_THROW(static_cast<void>(camera.position()), std::overflow_error);
    expectOrthonormalBasis(camera);
}

}
