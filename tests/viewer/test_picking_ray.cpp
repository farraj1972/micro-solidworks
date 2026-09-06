#include "viewer/PickingRay.h"
#include "viewer/GeometryPicker.h"
#include "viewer/ViewProjection.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace
{
using namespace microsw::viewer;
using microsw::ProjectionMode;
using microsw::geometry::Point3;
using microsw::math::Vector3;

PickingContext context()
{
    PickingContext result;
    result.width = 800;
    result.height = 600;
    result.mouseX = 400;
    result.mouseY = 300;
    return result;
}

void expectVector(const Vector3& a, const Vector3& b)
{
    EXPECT_NEAR(a.x(), b.x(), 1e-12);
    EXPECT_NEAR(a.y(), b.y(), 1e-12);
    EXPECT_NEAR(a.z(), b.z(), 1e-12);
}

Vector3 vector(const Point3& p) { return {p.x(), p.y(), p.z()}; }
Point3 point(const Vector3& v) { return {v.x(), v.y(), v.z()}; }

TEST(PickingRay, PerspectiveCenterUsesEyeAndForwardAcrossPoses)
{
    auto c = context();
    for (const auto& camera : {OrbitCamera{}, OrbitCamera{{3, -4, 2}, 20, 1.2, -0.5}})
    {
        c.camera = camera;
        const auto ray = makePickingRay(c);
        expectVector(vector(ray.origin()), camera.position());
        expectVector(ray.direction(), camera.forward());
        EXPECT_NEAR(ray.direction().length(), 1, 1e-12);
    }
}

TEST(PickingRay, PerspectiveEdgesRespectSignsFovAndAspect)
{
    auto c = context();
    for (double width : {300.0, 1200.0})
        for (double fov : {0.5, 1.5})
            for (const auto& mouse : std::array{
                     std::array{0.0, 0.5}, std::array{1.0, 0.5},
                     std::array{0.5, 0.0}, std::array{0.5, 1.0}})
            {
                c.width = width;
                c.verticalFov = fov;
                c.mouseX = mouse[0] * c.width;
                c.mouseY = mouse[1] * c.height;
                const auto ray = makePickingRay(c);
                const auto forward = microsw::math::dot(ray.direction(), c.camera.forward());
                ASSERT_GT(forward, 0);
                const auto expectedX = (2 * mouse[0] - 1) * std::tan(fov / 2) * width / c.height;
                const auto expectedY = (1 - 2 * mouse[1]) * std::tan(fov / 2);
                EXPECT_NEAR(microsw::math::dot(ray.direction(), c.camera.right()) / forward, expectedX, 1e-12);
                EXPECT_NEAR(microsw::math::dot(ray.direction(), c.camera.up()) / forward, expectedY, 1e-12);
                EXPECT_NEAR(ray.direction().length(), 1, 1e-12);
                expectVector(vector(ray.origin()), c.camera.position());
            }
}

TEST(PickingRay, OrthographicRaysAreParallelAndOriginsTrackZoomAspectAndSigns)
{
    auto c = context();
    c.projection.setMode(ProjectionMode::Orthographic);
    for (double height : {4.0, 20.0})
        for (double width : {300.0, 1200.0})
            for (const auto& mouse : std::array{
                     std::array{0.0, 0.5}, std::array{1.0, 0.5},
                     std::array{0.5, 0.0}, std::array{0.5, 1.0}, std::array{0.5, 0.5}})
            {
                c.projection.setVisibleHeight(height);
                c.width = width;
                c.mouseX = mouse[0] * width;
                c.mouseY = mouse[1] * c.height;
                const auto ray = makePickingRay(c);
                expectVector(ray.direction(), c.camera.forward());
                const auto expected = c.camera.position()
                    + c.camera.right() * ((mouse[0] - 0.5) * height * width / c.height)
                    + c.camera.up() * ((0.5 - mouse[1]) * height);
                expectVector(vector(ray.origin()), expected);
                EXPECT_NEAR(ray.direction().length(), 1, 1e-12);
            }
}

TEST(PickingProjection, TargetIsCenterAndBasisOffsetsHaveExpectedPixelSignsInBothModes)
{
    auto c = context();
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        const auto center = projectWorldToScreen(point(c.camera.target()), c);
        ASSERT_TRUE(center);
        EXPECT_NEAR(center->x, 400, 1e-10);
        EXPECT_NEAR(center->y, 300, 1e-10);
        EXPECT_NEAR(center->depth, c.camera.distance(), 1e-12);
        const auto worldHeight = mode == ProjectionMode::Perspective
            ? 2 * c.camera.distance() * std::tan(c.verticalFov / 2) : c.projection.visibleHeight();
        const auto sample = c.camera.target()
            + c.camera.right() * (50 * worldHeight / c.height)
            + c.camera.up() * (30 * worldHeight / c.height);
        const auto screen = projectWorldToScreen(point(sample), c);
        ASSERT_TRUE(screen);
        EXPECT_NEAR(screen->x, 450, 1e-10);
        EXPECT_NEAR(screen->y, 270, 1e-10);
    }
}

TEST(PickingProjection, ResizingChangesCenterAndPixelScaleConsistently)
{
    auto c = context();
    c.camera = OrbitCamera{{}, 10, 0, 0};
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        for (const auto& size : std::array{std::array{800.0, 600.0},
                                          std::array{1200.0, 600.0}, std::array{400.0, 900.0}})
        {
            c.width = size[0];
            c.height = size[1];
            const auto screen = projectWorldToScreen(Point3{0, 2, 1}, c);
            ASSERT_TRUE(screen);
            const auto worldHeight = mode == ProjectionMode::Perspective
                ? 20 * std::tan(c.verticalFov / 2) : c.projection.visibleHeight();
            EXPECT_NEAR(screen->x, c.width / 2 + 2 * c.height / worldHeight, 1e-10);
            EXPECT_NEAR(screen->y, c.height / 2 - c.height / worldHeight, 1e-10);
        }
    }
}

TEST(PickingProjection, RaysRoundTripToLogicalMouseAcrossCameraPosesAndModes)
{
    auto c = context();
    c.camera = OrbitCamera{{2, -3, 1}, 12, 0.8, -0.3};
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
        for (const auto& mouse : std::array{std::array{100.0, 200.0}, std::array{650.0, 450.0}})
        {
            c.projection.setMode(mode);
            c.mouseX = mouse[0];
            c.mouseY = mouse[1];
            const auto ray = makePickingRay(c);
            const auto screen = projectWorldToScreen(ray.pointAt(20), c);
            ASSERT_TRUE(screen);
            EXPECT_NEAR(screen->x, c.mouseX, 1e-9);
            EXPECT_NEAR(screen->y, c.mouseY, 1e-9);
        }
}

TEST(PickingProjection, RejectsBehindEyeNearFarAndOutsideSidePlanes)
{
    auto c = context();
    c.camera = OrbitCamera{{}, 10, 0, 0};
    c.nearPlane = 1;
    c.farPlane = 20;
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        for (const auto& p : {Point3{11, 0, 0}, Point3{10, 0, 0}, Point3{9.5, 0, 0},
                              Point3{-11, 0, 0}, Point3{0, 100, 0}, Point3{0, 0, -100}})
            EXPECT_FALSE(projectWorldToScreen(p, c));
        EXPECT_TRUE(projectWorldToScreen(Point3{}, c));
    }
}

TEST(PickingContext, RejectsNonFiniteAndInvalidParameters)
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    for (double value : {nan, inf, -inf})
        for (auto member : {&PickingContext::mouseX, &PickingContext::mouseY,
                            &PickingContext::width, &PickingContext::height,
                            &PickingContext::verticalFov, &PickingContext::nearPlane,
                            &PickingContext::farPlane, &PickingContext::tolerancePixels})
        {
            auto c = context();
            c.*member = value;
            EXPECT_THROW((void)makePickingRay(c), std::invalid_argument);
            EXPECT_THROW((void)projectWorldToScreen(Point3{}, c), std::invalid_argument);
        }
    for (double value : {0.0, -1.0})
        for (auto member : {&PickingContext::width, &PickingContext::height,
                            &PickingContext::verticalFov, &PickingContext::nearPlane,
                            &PickingContext::farPlane, &PickingContext::tolerancePixels})
        {
            auto c = context();
            c.*member = value;
            EXPECT_THROW(c.validate(), std::invalid_argument);
        }
    auto c = context();
    c.verticalFov = std::numbers::pi;
    EXPECT_THROW(c.validate(), std::invalid_argument);
    c = context();
    c.farPlane = c.nearPlane;
    EXPECT_THROW(c.validate(), std::invalid_argument);
    c = context();
    c.projection.setMode(static_cast<ProjectionMode>(42));
    EXPECT_THROW(c.validate(), std::invalid_argument);
    for (double height : {0.0, -1.0, nan, inf})
        EXPECT_THROW(c.projection.setVisibleHeight(height), std::invalid_argument);
}

TEST(PickingContext, RejectsUnrepresentableAspectAndProjection)
{
    auto c = context();
    c.width = std::numeric_limits<double>::max();
    c.height = std::numeric_limits<double>::min();
    EXPECT_THROW(c.validate(), std::invalid_argument);
    std::swap(c.width, c.height);
    EXPECT_THROW(c.validate(), std::invalid_argument);
    c = context();
    c.verticalFov = std::numeric_limits<double>::denorm_min();
    EXPECT_THROW(c.validate(), std::overflow_error);
}

TEST(PickingRay, ReportsRepresentationalOverflowWithoutReturningNonFiniteRay)
{
    auto c = context();
    c.mouseX = std::numeric_limits<double>::max();
    c.width = 1;
    EXPECT_THROW((void)makePickingRay(c), std::overflow_error);
    c = context();
    c.camera = OrbitCamera{{std::numeric_limits<double>::max(), 0, 0},
        std::numeric_limits<double>::max(), 0, 0};
    EXPECT_THROW((void)makePickingRay(c), std::overflow_error);
    EXPECT_THROW((void)projectWorldToScreen(Point3{}, c), std::overflow_error);
}

TEST(PickingProjection, MatchesExistingMatrixPipelineForBothModes)
{
    auto c = context();
    c.camera = OrbitCamera{{3, 4, -2}, 15, 1.2, 0.4};
    const auto p = c.camera.target() + c.camera.right() + c.camera.up() * 0.5;
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        const auto matrix = c.projection.matrix(c.verticalFov, c.width / c.height, c.nearPlane, c.farPlane)
            * viewMatrix(c.camera);
        const std::array<double, 4> world{p.x(), p.y(), p.z(), 1};
        std::array<double, 4> clip{};
        for (std::size_t row = 0; row < 4; ++row)
            for (std::size_t column = 0; column < 4; ++column)
                clip[row] += matrix(row, column) * world[column];
        const auto screen = projectWorldToScreen(point(p), c);
        ASSERT_TRUE(screen);
        EXPECT_NEAR(screen->x, (clip[0] / clip[3] + 1) * c.width / 2, 1e-9);
        EXPECT_NEAR(screen->y, (1 - clip[1] / clip[3]) * c.height / 2, 1e-9);
    }
}
TEST(PickingContext, ExplicitRasterAspectIsValidated)
{
    auto c = context();
    for (double aspect : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()})
    {
        c.projectionAspectRatio = aspect;
        EXPECT_THROW(c.validate(), std::invalid_argument);
    }
}

TEST(PickingRay, RasterAspectChangesFrustumWhileMouseStaysInLogicalUnits)
{
    auto c = context();
    c.projectionAspectRatio = 2;
    c.mouseX = 600;
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        const auto ray = makePickingRay(c);
        const auto screen = projectWorldToScreen(ray.pointAt(20), c);
        ASSERT_TRUE(screen);
        EXPECT_NEAR(screen->x, 600, 1e-9);
        EXPECT_NEAR(screen->y, 300, 1e-9);
        if (mode == ProjectionMode::Perspective)
        {
            const auto ratio = microsw::math::dot(ray.direction(), c.camera.right())
                / microsw::math::dot(ray.direction(), c.camera.forward());
            EXPECT_NEAR(ratio, std::tan(c.verticalFov / 2), 1e-12);
        }
        else
            expectVector(vector(ray.origin()), c.camera.position()
                + c.camera.right() * (c.projection.visibleHeight() / 2));
    }
}
}
