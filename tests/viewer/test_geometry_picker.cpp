#include "viewer/GeometryPicker.h"
#include "viewer/PresentedLines.h"
#include "app/demo/GeometryDemoScene.h"
#include "presentation/GeometryPresentation.h"
#include "core/geometry/GeometricTolerance.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using namespace microsw::viewer;
using microsw::ProjectionMode;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::Line3;
using microsw::math::Vector3;
using microsw::presentation::GeometryPresentation;

PickingContext context()
{
    PickingContext c;
    // Axis-aligned oracle: eye (10,0,0), forward -X, right +Y, up +Z.
    c.camera = OrbitCamera{{}, 10, 0, 0};
    c.width = 800;
    c.height = 600;
    c.mouseX = 400;
    c.mouseY = 300;
    return c;
}

// Independent analytic construction for the axis-aligned camera above.
// Does not use picking rays or the projection helper to place test geometry.
Point3 world(const PickingContext& c, double x, double y, double depth = 10)
{
    const auto unitsPerPixel = c.projection.mode() == ProjectionMode::Perspective
        ? 2 * depth * std::tan(c.verticalFov / 2) / c.height
        : c.projection.visibleHeight() / c.height;
    return {10 - depth, (x - c.width / 2) * unitsPerPixel,
            (c.height / 2 - y) * unitsPerPixel};
}

TEST(GeometryPicker, EmptyAndOutsideHalfOpenViewportReturnNoHit)
{
    auto c = context();
    GeometryPresentation presentation;
    EXPECT_FALSE(pickGeometry(presentation, c));
    (void)presentation.add(Point3{});
    for (const auto& mouse : std::array{std::array{-0.01, 300.0}, std::array{800.0, 300.0},
                                      std::array{400.0, -0.01}, std::array{400.0, 600.0}})
    {
        c.mouseX = mouse[0];
        c.mouseY = mouse[1];
        EXPECT_FALSE(pickGeometry(presentation, c));
    }
    c = context();
    GeometryPresentation edge;
    const auto id = edge.add(world(c, 0, 300));
    c.mouseX = 0;
    const auto hit = pickGeometry(edge, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, id);
}

TEST(GeometryPicker, PointUsesInclusivePixelToleranceAndReturnsVisualId)
{
    auto c = context();
    GeometryPresentation presentation;
    const auto id = presentation.add(Point3{});
    for (double offset : {0.0, 4.0, 6.0})
    {
        c.mouseX = 400 + offset;
        const auto hit = pickGeometry(presentation, c);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, id);
        EXPECT_NEAR(hit->screenDistance, offset, 1e-10);
    }
    c.mouseX = 406.0001;
    EXPECT_FALSE(pickGeometry(presentation, c));
    c.tolerancePixels = 8;
    EXPECT_TRUE(pickGeometry(presentation, c));
}

TEST(GeometryPicker, NearestPointWinsBeforeDepthAndDepthBreaksVisualTies)
{
    auto c = context();
    GeometryPresentation presentation;
    (void)presentation.add(world(c, 403, 300, 2));
    const auto fartherButCentered = presentation.add(world(c, 400, 300, 20));
    auto hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, fartherButCentered);
    const auto nearerAndCentered = presentation.add(world(c, 400, 300, 5));
    hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, nearerAndCentered);
}

TEST(GeometryPicker, SubpixelDistanceTieUsesDepthDeterministically)
{
    auto c = context();
    GeometryPresentation presentation;
    (void)presentation.add(world(c, 400, 300, 20));
    const auto near = presentation.add(world(c, 400 + 5e-8, 300, 5));
    const auto hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, near);
}

TEST(GeometryPicker, BehindAndOutsidePointsCannotWin)
{
    auto c = context();
    GeometryPresentation presentation;
    (void)presentation.add(Point3{12, 0, 0});
    (void)presentation.add(Point3{9.99, 0, 0});
    (void)presentation.add(Point3{-2000, 0, 0});
    (void)presentation.add(world(c, -2, 300));
    c.mouseX = 0;
    EXPECT_FALSE(pickGeometry(presentation, c));
    c.mouseX = 400;
    EXPECT_FALSE(pickGeometry(presentation, c));
    const auto visible = presentation.add(Point3{});
    const auto hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, visible);
}

TEST(GeometryPicker, SegmentMidpointEndpointsDiagonalAndVerticalUseScreenDistance)
{
    auto c = context();
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        for (const auto& endpoints : std::array{
                 std::array{300.0, 300.0, 500.0, 300.0},
                 std::array{400.0, 200.0, 400.0, 400.0},
                 std::array{300.0, 200.0, 500.0, 400.0}})
        {
            GeometryPresentation presentation;
            const auto id = presentation.add(Segment3{
                world(c, endpoints[0], endpoints[1]), world(c, endpoints[2], endpoints[3])});
            for (double t : {0.0, 0.5, 1.0})
            {
                c.mouseX = std::lerp(endpoints[0], endpoints[2], t);
                c.mouseY = std::lerp(endpoints[1], endpoints[3], t);
                const auto hit = pickGeometry(presentation, c);
                ASSERT_TRUE(hit);
                EXPECT_EQ(hit->id, id);
                EXPECT_NEAR(hit->screenDistance, 0, 1e-9);
            }
            const auto dx = endpoints[2] - endpoints[0];
            const auto dy = endpoints[3] - endpoints[1];
            const auto length = std::hypot(dx, dy);
            for (double offset : {4.0, 7.0})
            {
                c.mouseX = 400 - dy / length * offset;
                c.mouseY = 300 + dx / length * offset;
                EXPECT_EQ(pickGeometry(presentation, c).has_value(), offset < 6);
            }
        }
    }
}

TEST(GeometryPicker, SegmentEndpointToleranceDoesNotExtendToInfiniteSupport)
{
    auto c = context();
    GeometryPresentation presentation;
    (void)presentation.add(Segment3{world(c, 300, 300), world(c, 500, 300)});
    c.mouseX = 504;
    EXPECT_TRUE(pickGeometry(presentation, c));
    c.mouseX = 507;
    EXPECT_FALSE(pickGeometry(presentation, c));
}

TEST(GeometryPicker, DegenerateAndViewAlignedSegmentsBehaveAsPoints)
{
    auto c = context();
    for (const auto& segment : {Segment3{Point3{}, Point3{}},
                                Segment3{Point3{}, Point3{5, 0, 0}}})
    {
        GeometryPresentation presentation;
        (void)presentation.add(world(c, 400, 300, 8));
        const auto segmentId = presentation.add(segment);
        const auto hit = pickGeometry(presentation, c);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, segment.isDegenerate() ? presentation.entities()[0].id() : segmentId);
        c.mouseY = 307;
        EXPECT_FALSE(pickGeometry(presentation, c));
        c.mouseY = 300;
    }
}

TEST(GeometryPicker, ClipsSegmentsCrossingViewportWithBothEndpointsOutside)
{
    auto c = context();
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        GeometryPresentation presentation;
        const auto id = presentation.add(Segment3{world(c, -200, 300), world(c, 1000, 300)});
        const auto hit = pickGeometry(presentation, c);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, id);
        c.mouseY = 307;
        EXPECT_FALSE(pickGeometry(presentation, c));
        c.mouseY = 300;
        GeometryPresentation offscreen;
        (void)offscreen.add(Segment3{world(c, -100, 100), world(c, -100, 500)});
        EXPECT_FALSE(pickGeometry(offscreen, c));
    }
}

TEST(GeometryPicker, ClipsAcrossNearPlaneAndRejectsEntirelyBehindSegments)
{
    auto c = context();
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        GeometryPresentation crossing;
        const auto id = crossing.add(Segment3{Point3{12, -2, 0}, Point3{0, 2, 0}});
        const auto hit = pickGeometry(crossing, c);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, id);
        GeometryPresentation behind;
        (void)behind.add(Segment3{Point3{12, -2, 0}, Point3{20, 2, 0}});
        EXPECT_FALSE(pickGeometry(behind, c));
    }
}

TEST(GeometryPicker, FarClippingExcludesInvisiblePortionOfSegment)
{
    auto c = context();
    c.farPlane = 20;
    GeometryPresentation presentation;
    (void)presentation.add(Segment3{world(c, 100, 300, 5), world(c, 700, 300, 50)});
    c.mouseX = 500;
    EXPECT_TRUE(pickGeometry(presentation, c));
    c.mouseX = 650;
    EXPECT_FALSE(pickGeometry(presentation, c));
}

TEST(GeometryPicker, PerspectiveSegmentDepthUsesReciprocalInterpolation)
{
    auto c = context();
    GeometryPresentation presentation;
    (void)presentation.add(world(c, 400, 300, 5));
    const auto segment = presentation.add(Segment3{
        world(c, 100, 300, 2), world(c, 700, 300, 20)});
    auto hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    // Midpoint on screen has depth 1/(0.5/2 + 0.5/20), about 3.636, not 11.
    EXPECT_EQ(hit->id, segment);
    const auto nearerPoint = presentation.add(world(c, 400, 300, 3));
    hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, nearerPoint);
}

TEST(GeometryPicker, OrthographicSegmentDepthUsesLinearInterpolation)
{
    auto c = context();
    c.projection.setMode(ProjectionMode::Orthographic);
    GeometryPresentation presentation;
    const auto point = presentation.add(world(c, 400, 300, 5));
    (void)presentation.add(Segment3{world(c, 100, 300, 2), world(c, 700, 300, 20)});
    const auto hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, point);
}

TEST(GeometryPicker, LineTracksPanZoomProjectionAndAspectWithPixelTolerance)
{
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
        for (double zoom : {5.0, 20.0, 80.0})
            for (double width : {300.0, 1200.0})
            {
                auto c = context();
                c.width = width;
                c.mouseX = width / 2;
                c.projection.setMode(mode);
                c.projection.setVisibleHeight(zoom);
                c.camera.setDistance(zoom);
                c.camera.setTarget({0, 15, 0});
                GeometryPresentation presentation;
                const auto id = presentation.add(Line3{Point3{}, Vector3{0, 1, 0}});
                c.mouseY = 304;
                const auto hit = pickGeometry(presentation, c);
                ASSERT_TRUE(hit);
                EXPECT_EQ(hit->id, id);
                EXPECT_NEAR(hit->screenDistance, 4, 1e-9);
                c.mouseY = 307;
                EXPECT_FALSE(pickGeometry(presentation, c));
            }
}

TEST(GeometryPicker, LineIsLimitedToProductionAdaptersFiniteVisualExtent)
{
    auto c = context();
    GeometryPresentation presentation;
    const auto id = presentation.add(Line3{Point3{-490, 0, 0}, Vector3{0, 1, 0}});
    const auto scale = 20 * std::tan(c.verticalFov / 2) * c.width / c.height;
    const auto vertices = presentedLineVertices(presentation, {c.camera.target(), scale});
    ASSERT_EQ(vertices.size(), 2U);
    const auto endpoint = projectWorldToScreen(
        Point3{vertices[1].x(), vertices[1].y(), vertices[1].z()}, c);
    ASSERT_TRUE(endpoint);
    c.mouseX = endpoint->x + 4;
    const auto hit = pickGeometry(presentation, c);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, id);
    c.mouseX = endpoint->x + 7;
    EXPECT_FALSE(pickGeometry(presentation, c));
    c.mouseX = 600;
    EXPECT_FALSE(pickGeometry(presentation, c)); // Infinite support would falsely hit.
}

TEST(GeometryPicker, PanMovesLineExtentAlongSupportWithoutPickingOldInvisiblePart)
{
    auto c = context();
    GeometryPresentation presentation;
    (void)presentation.add(Line3{Point3{-490, 0, 0}, Vector3{0, 1, 0}});
    EXPECT_TRUE(pickGeometry(presentation, c));
    c.camera.setTarget({0, 200, 0});
    const auto oldCenter = projectWorldToScreen(Point3{-490, 0, 0}, c);
    ASSERT_TRUE(oldCenter);
    c.mouseX = oldCenter->x;
    EXPECT_FALSE(pickGeometry(presentation, c));
    c.mouseX = 400;
    EXPECT_TRUE(pickGeometry(presentation, c));
}

TEST(GeometryPicker, MixedTypesRankByVisualDistanceWithoutTypePriority)
{
    auto c = context();
    GeometryPresentation presentation;
    const auto point = presentation.add(world(c, 400, 304));
    const auto segment = presentation.add(Segment3{world(c, 300, 302), world(c, 500, 302)});
    const auto line = presentation.add(Line3{Point3{}, Vector3{0, 1, 0}});
    const std::array ids{line, segment, point};
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        c.mouseY = 300 + 2.0 * i;
        const auto hit = pickGeometry(presentation, c);
        ASSERT_TRUE(hit);
        EXPECT_EQ(hit->id, ids[i]);
    }
}

TEST(GeometryPicker, EqualDistanceAndDepthRetainInsertionOrderAcrossTypes)
{
    auto c = context();
    for (bool lineFirst : {false, true})
    {
        GeometryPresentation presentation;
        if (lineFirst) (void)presentation.add(Line3{Point3{}, Vector3{0, 1, 0}});
        (void)presentation.add(Point3{});
        (void)presentation.add(Segment3{world(c, 300, 300), world(c, 500, 300)});
        if (!lineFirst) (void)presentation.add(Line3{Point3{}, Vector3{0, 1, 0}});
        for (int repeat = 0; repeat < 3; ++repeat)
        {
            const auto hit = pickGeometry(presentation, c);
            ASSERT_TRUE(hit);
            EXPECT_EQ(hit->id, presentation.entities().front().id());
        }
    }
}

TEST(GeometryPicker, ToleranceIsIndependentOfGeometryAndStableAcrossZoomAndModes)
{
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
        for (double zoom : {5.0, 20.0, 80.0})
        {
            auto c = context();
            c.projection.setMode(mode);
            c.projection.setVisibleHeight(zoom);
            c.camera.setDistance(zoom);
            GeometryPresentation presentation;
            const auto id = presentation.add(Point3{});
            c.mouseX = 404;
            const auto hit = pickGeometry(presentation, c);
            ASSERT_TRUE(hit);
            EXPECT_EQ(hit->id, id);
            EXPECT_NEAR(hit->screenDistance, 4, 1e-9);
            const auto worldHeight = mode == ProjectionMode::Perspective
                ? 2 * zoom * std::tan(c.verticalFov / 2) : zoom;
            EXPECT_GT(4 * worldHeight / c.height, microsw::geometry::defaultGeometricTolerance * 1e6);
            EXPECT_DOUBLE_EQ(defaultPickingTolerancePixels, 6);
            c.mouseX = 407;
            EXPECT_FALSE(pickGeometry(presentation, c));
        }
}

TEST(GeometryPicker, RealDemoReturnsOriginPointWithoutMutatingPresentation)
{
    auto c = context();
    c.camera = OrbitCamera{};
    const auto presentation = microsw::demo::createGeometryDemoScene();
    const auto first = pickGeometry(presentation, c);
    ASSERT_TRUE(first);
    EXPECT_EQ(first->id, presentation.entities().front().id());
    EXPECT_EQ(presentation.size(), 8U);
    const auto second = pickGeometry(presentation, c);
    ASSERT_TRUE(second);
    EXPECT_EQ(second->id, first->id);
    EXPECT_DOUBLE_EQ(second->screenDistance, first->screenDistance);
}

TEST(GeometryPicker, InvalidInputsThrowEvenForEmptyCollectionWithoutNaNLeakage)
{
    const GeometryPresentation empty;
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    for (double value : {nan, inf, -inf})
        for (auto member : {&PickingContext::mouseX, &PickingContext::mouseY,
                            &PickingContext::width, &PickingContext::height,
                            &PickingContext::tolerancePixels})
        {
            auto c = context();
            c.*member = value;
            EXPECT_THROW((void)pickGeometry(empty, c), std::invalid_argument);
        }
    auto c = context();
    c.width = 0;
    EXPECT_THROW((void)pickGeometry(empty, c), std::invalid_argument);
    c = context();
    c.tolerancePixels = 0;
    EXPECT_THROW((void)pickGeometry(empty, c), std::invalid_argument);
}

TEST(GeometryPicker, UnrepresentableProjectionThrowsInsteadOfReturningNonFiniteHit)
{
    auto c = context();
    c.verticalFov = 0.01;
    GeometryPresentation presentation;
    (void)presentation.add(Point3{0, std::numeric_limits<double>::max(), 0});
    EXPECT_THROW((void)pickGeometry(presentation, c), std::overflow_error);
}
TEST(GeometryPicker, PointOnlyCollectionDoesNotRequireRepresentableLineExtent)
{
    auto c = context();
    c.camera.setDistance(1e308);
    GeometryPresentation presentation;
    (void)presentation.add(Point3{});
    EXPECT_FALSE(pickGeometry(presentation, c)); // Beyond far plane, not a Line overflow.
}

TEST(GeometryPicker, ClipBoundaryRoundoffTreatsPointsAndDegenerateSegmentsConsistently)
{
    auto c = context();
    for (auto mode : {ProjectionMode::Perspective, ProjectionMode::Orthographic})
    {
        c.projection.setMode(mode);
        for (const auto& mouse : std::array{std::array{0.0, 300.0}, std::array{400.0, 0.0}})
        {
            c.mouseX = mouse[0];
            c.mouseY = mouse[1];
            const auto p = world(c, c.mouseX, c.mouseY);
            GeometryPresentation points, segments;
            (void)points.add(p);
            (void)segments.add(Segment3{p, p});
            EXPECT_TRUE(pickGeometry(points, c));
            EXPECT_TRUE(pickGeometry(segments, c));
        }
    }
}
}
