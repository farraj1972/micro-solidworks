#include "viewer/ViewProjection.h"
#include "viewer/OrbitCamera.h"
#include "core/math/Transformations.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{

using namespace microsw::math;
using namespace microsw::viewer;

constexpr Scalar pi = std::numbers::pi_v<Scalar>;
const std::array<Scalar, 3> nonFinite{
    std::numeric_limits<Scalar>::quiet_NaN(),
    std::numeric_limits<Scalar>::infinity(),
    -std::numeric_limits<Scalar>::infinity()};

// Test-only homogeneous multiplication and divide; B1 affine helpers are unchanged.
struct HomogeneousPoint
{
    Scalar x, y, z, w;
};

HomogeneousPoint clipPoint(const Matrix4& m, const Vector3& p)
{
    return {
        m(0, 0)*p.x() + m(0, 1)*p.y() + m(0, 2)*p.z() + m(0, 3),
        m(1, 0)*p.x() + m(1, 1)*p.y() + m(1, 2)*p.z() + m(1, 3),
        m(2, 0)*p.x() + m(2, 1)*p.y() + m(2, 2)*p.z() + m(2, 3),
        m(3, 0)*p.x() + m(3, 1)*p.y() + m(3, 2)*p.z() + m(3, 3)};
}

Vector3 ndc(const Matrix4& m, const Vector3& p)
{
    const auto c = clipPoint(m, p);
    EXPECT_TRUE(std::isfinite(c.w));
    EXPECT_NE(c.w, 0.0);
    return {c.x/c.w, c.y/c.w, c.z/c.w};
}

const std::array<OrbitCamera, 4> cameras{
    OrbitCamera{},
    OrbitCamera{Vector3{3.0, -4.0, 2.0}, 7.0, 0.6, 0.4},
    OrbitCamera{Vector3{-2.0, 1.0, -3.0}, 12.0, -1.2, -0.7},
    OrbitCamera{Vector3{}, 10.0, 0.0, 0.0}};

TEST(ViewProjection, ViewMapsEyeToOrigin)
{
    for (const auto& camera : cameras)
        EXPECT_TRUE(almostEqual(transformPoint(viewMatrix(camera), camera.position()), Vector3{}));
}

TEST(ViewProjection, ViewMapsRightToPositiveX)
{
    for (const auto& camera : cameras)
        EXPECT_TRUE(almostEqual(transformDirection(viewMatrix(camera), camera.right()), Vector3{1.0, 0.0, 0.0}));
}

TEST(ViewProjection, ViewMapsUpToPositiveY)
{
    for (const auto& camera : cameras)
        EXPECT_TRUE(almostEqual(transformDirection(viewMatrix(camera), camera.up()), Vector3{0.0, 1.0, 0.0}));
}

TEST(ViewProjection, ViewMapsForwardToNegativeZ)
{
    for (const auto& camera : cameras)
        EXPECT_TRUE(almostEqual(transformDirection(viewMatrix(camera), camera.forward()), Vector3{0.0, 0.0, -1.0}));
}

TEST(ViewProjection, ViewMapsTargetToNegativeDistance)
{
    for (const auto& camera : cameras)
        EXPECT_TRUE(almostEqual(transformPoint(viewMatrix(camera), camera.target()), Vector3{0.0, 0.0, -camera.distance()}));
}

TEST(ViewProjection, ViewPreservesCameraState)
{
    OrbitCamera camera{Vector3{1.0, 2.0, 3.0}, 9.0, 0.4, 0.7};
    static_cast<void>(viewMatrix(camera));
    EXPECT_TRUE(almostEqual(camera.target(), Vector3{1.0, 2.0, 3.0}));
    EXPECT_TRUE(almostEqual(camera.distance(), 9.0));
    EXPECT_TRUE(almostEqual(camera.yaw(), 0.4));
    EXPECT_TRUE(almostEqual(camera.pitch(), 0.7));
}

TEST(ViewProjection, ViewRemainsValidNearBothPoles)
{
    for (const Scalar pitch : {-pi, pi})
    {
        const OrbitCamera camera{Vector3{1.0, 2.0, 3.0}, 10.0, 0.7, pitch};
        const auto view = viewMatrix(camera);
        EXPECT_TRUE(almostEqual(transformPoint(view, camera.position()), Vector3{}));
        EXPECT_TRUE(almostEqual(transformDirection(view, camera.forward()), Vector3{0.0, 0.0, -1.0}));
        EXPECT_TRUE(almostEqual(transformDirection(view, camera.right()), Vector3{1.0, 0.0, 0.0}));
        EXPECT_TRUE(almostEqual(transformDirection(view, camera.up()), Vector3{0.0, 1.0, 0.0}));
    }
}

TEST(ViewProjection, PerspectiveRejectsInvalidFov)
{
    for (const Scalar fov : {0.0, -0.1, pi, pi + 0.1})
        EXPECT_THROW(static_cast<void>(perspective(fov, 1.0, 0.1, 100.0)), std::invalid_argument);
}

TEST(ViewProjection, PerspectiveRejectsInvalidAspect)
{
    for (const Scalar aspect : {0.0, -1.0})
        EXPECT_THROW(static_cast<void>(perspective(pi/3.0, aspect, 0.1, 100.0)), std::invalid_argument);
}

TEST(ViewProjection, PerspectiveRejectsInvalidDepth)
{
    for (const auto bounds : {std::array<Scalar, 2>{0.0, 10.0}, {-1.0, 10.0}, {1.0, 1.0}, {2.0, 1.0}})
        EXPECT_THROW(static_cast<void>(perspective(pi/3.0, 1.0, bounds[0], bounds[1])), std::invalid_argument);
}

TEST(ViewProjection, PerspectiveRejectsEveryNonFiniteParameter)
{
    for (const Scalar value : nonFinite)
    {
        EXPECT_THROW(static_cast<void>(perspective(value, 1.0, 0.1, 100.0)), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(perspective(pi/3.0, value, 0.1, 100.0)), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(perspective(pi/3.0, 1.0, value, 100.0)), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(perspective(pi/3.0, 1.0, 0.1, value)), std::invalid_argument);
    }
}

TEST(ViewProjection, PerspectiveMapsNearAndFarToNdcDepth)
{
    const auto p = perspective(pi/3.0, 1.6, 0.5, 50.0);
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -0.5}).z(), -1.0));
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -50.0}).z(), 1.0));
    EXPECT_TRUE(almostEqual(clipPoint(p, {0.0, 0.0, -5.0}).w, 5.0));
}

TEST(ViewProjection, PerspectiveCenterRemainsCentered)
{
    const auto p = perspective(pi/3.0, 1.6, 0.5, 50.0);
    for (const Scalar z : {0.5, 5.0, 50.0})
    {
        const auto result = ndc(p, {0.0, 0.0, -z});
        EXPECT_TRUE(almostEqual(result.x(), 0.0));
        EXPECT_TRUE(almostEqual(result.y(), 0.0));
    }
}

TEST(ViewProjection, PerspectiveAspectOnlyChangesHorizontalScale)
{
    const auto square = perspective(pi/3.0, 1.0, 0.5, 50.0);
    const auto wide = perspective(pi/3.0, 2.0, 0.5, 50.0);
    EXPECT_LT(wide(0, 0), square(0, 0));
    EXPECT_TRUE(almostEqual(wide(0, 0)*2.0, square(0, 0)));
    EXPECT_TRUE(almostEqual(wide(1, 1), square(1, 1)));
}

TEST(ViewProjection, PerspectiveVerticalFovDeterminesVisibleBounds)
{
    for (const Scalar fov : {pi/3.0, pi/2.0})
    {
        const auto p = perspective(fov, 2.0, 0.5, 50.0);
        const Scalar halfHeight = 5.0 * std::tan(fov/2.0);
        EXPECT_TRUE(almostEqual(ndc(p, {0.0, halfHeight, -5.0}).y(), 1.0));
        EXPECT_TRUE(almostEqual(ndc(p, {2.0*halfHeight, 0.0, -5.0}).x(), 1.0));
    }
    EXPECT_LT(perspective(pi/2.0, 1.0, 0.5, 50.0)(1, 1),
              perspective(pi/3.0, 1.0, 0.5, 50.0)(1, 1));
}

TEST(ViewProjection, OrthographicRejectsInvalidHeight)
{
    for (const Scalar height : {0.0, -1.0})
        EXPECT_THROW(static_cast<void>(orthographic(height, 1.0, 0.1, 100.0)), std::invalid_argument);
}

TEST(ViewProjection, OrthographicRejectsInvalidAspect)
{
    for (const Scalar aspect : {0.0, -1.0})
        EXPECT_THROW(static_cast<void>(orthographic(10.0, aspect, 0.1, 100.0)), std::invalid_argument);
}

TEST(ViewProjection, OrthographicRejectsInvalidDepth)
{
    for (const auto bounds : {std::array<Scalar, 2>{0.0, 10.0}, {-1.0, 10.0}, {1.0, 1.0}, {2.0, 1.0}})
        EXPECT_THROW(static_cast<void>(orthographic(10.0, 1.0, bounds[0], bounds[1])), std::invalid_argument);
}

TEST(ViewProjection, OrthographicRejectsEveryNonFiniteParameter)
{
    for (const Scalar value : nonFinite)
    {
        EXPECT_THROW(static_cast<void>(orthographic(value, 1.0, 0.1, 100.0)), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(orthographic(10.0, value, 0.1, 100.0)), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(orthographic(10.0, 1.0, value, 100.0)), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(orthographic(10.0, 1.0, 0.1, value)), std::invalid_argument);
    }
}

TEST(ViewProjection, OrthographicMapsSymmetricBounds)
{
    const auto p = orthographic(8.0, 2.0, 1.0, 11.0);
    for (const Scalar sign : {-1.0, 1.0})
    {
        EXPECT_TRUE(almostEqual(ndc(p, {sign*8.0, 0.0, -6.0}).x(), sign));
        EXPECT_TRUE(almostEqual(ndc(p, {0.0, sign*4.0, -6.0}).y(), sign));
    }
}

TEST(ViewProjection, OrthographicMapsNearFarAndVolumeCenter)
{
    const auto p = orthographic(8.0, 2.0, 1.0, 11.0);
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -1.0}).z(), -1.0));
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -11.0}).z(), 1.0));
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -6.0}), Vector3{}));
    EXPECT_TRUE(almostEqual(clipPoint(p, {2.0, 3.0, -6.0}).w, 1.0));
}

TEST(ViewProjection, ProjectionViewCompositionCentersDefaultTarget)
{
    const OrbitCamera camera;
    const auto view = viewMatrix(camera);
    EXPECT_LT(transformPoint(view, camera.target()).z(), 0.0);
    for (const auto projection : {perspective(pi/3.0, 1.6, 0.1, 100.0),
                                  orthographic(10.0, 1.6, 0.1, 100.0)})
    {
        // Column vectors: Model = Identity, so apply View first, Projection second.
        const auto combined = projection * view * Matrix4::identity();
        const auto clip = clipPoint(combined, camera.target());
        EXPECT_GT(clip.w, 0.0);
        EXPECT_TRUE(std::isfinite(clip.x));
        EXPECT_TRUE(std::isfinite(clip.y));
        EXPECT_TRUE(std::isfinite(clip.z));
        EXPECT_TRUE(std::isfinite(clip.w));
        const auto result = ndc(combined, camera.target());
        EXPECT_TRUE(almostEqual(result.x(), 0.0));
        EXPECT_TRUE(almostEqual(result.y(), 0.0));
        EXPECT_TRUE(std::isfinite(result.z()));
        EXPECT_GT(result.z(), -1.0);
        EXPECT_LT(result.z(), 1.0);
        // An off-axis point also detects an incorrectly ordered product.
        const auto point = camera.target() + camera.right() * 2.0 + camera.up();
        EXPECT_TRUE(almostEqual(ndc(combined, point),
                                ndc(projection, transformPoint(view, point))));
    }
}

TEST(ViewProjection, UnrepresentableProjectionCoefficientsFailExplicitly)
{
    const Scalar tiny = std::numeric_limits<Scalar>::denorm_min();
    EXPECT_THROW(static_cast<void>(perspective(tiny, 1.0, 0.1, 100.0)), std::overflow_error);
    EXPECT_THROW(static_cast<void>(orthographic(tiny, 1.0, 0.1, 100.0)), std::overflow_error);
}

TEST(ViewProjection, LargeDepthBoundsAvoidIntermediateProductOverflow)
{
    const auto p = perspective(pi/3.0, 1.0, 1.0e150, 1.0e160);
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -1.0e150}).z(), -1.0));
    EXPECT_TRUE(almostEqual(ndc(p, {0.0, 0.0, -1.0e160}).z(), 1.0));
}

}
