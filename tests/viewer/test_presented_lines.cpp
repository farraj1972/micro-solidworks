#include "viewer/PresentedLines.h"

#include "core/geometry/GeometricTolerance.h"
#include "presentation/GeometryPresentation.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::math::Vector3;
using microsw::presentation::GeometryPresentation;
using microsw::viewer::LinePresentationContext;
using microsw::viewer::presentedLineVertices;

void expectVectorNear(const Vector3& actual, const Vector3& expected, double tolerance = 1e-12)
{
    EXPECT_NEAR(actual.x(), expected.x(), tolerance);
    EXPECT_NEAR(actual.y(), expected.y(), tolerance);
    EXPECT_NEAR(actual.z(), expected.z(), tolerance);
}

TEST(PresentedLines, EmptyPointAndSegmentPresentationsProduceNoVertices)
{
    GeometryPresentation presentation;
    EXPECT_TRUE(presentedLineVertices(presentation, {{}, 2}).empty());
    (void)presentation.add(Point3{1, 2, 3});
    (void)presentation.add(Segment3{Point3{}, Point3{1, 0, 0}});
    EXPECT_TRUE(presentedLineVertices(presentation, {{}, 2}).empty());
}

TEST(PresentedLines, AxisLineIsCenteredOnNearestViewPointWithExpectedExtent)
{
    GeometryPresentation presentation;
    const Line3 line{Point3{5, 2, 3}, Vector3{1, 0, 0}};
    (void)presentation.add(line);
    const auto vertices = presentedLineVertices(presentation, {{10, 9, 8}, 2});
    ASSERT_EQ(vertices.size(), 2U);
    expectVectorNear(vertices[0], {4, 2, 3});
    expectVectorNear(vertices[1], {16, 2, 3});
    EXPECT_TRUE(line.contains(Point3{vertices[0].x(), vertices[0].y(), vertices[0].z()}, 1e-12));
    EXPECT_TRUE(line.contains(Point3{vertices[1].x(), vertices[1].y(), vertices[1].z()}, 1e-12));
}

TEST(PresentedLines, DiagonalLineEndpointsBelongAndMidpointIsClosestToViewCenter)
{
    GeometryPresentation presentation;
    const Line3 line{Point3{-2, 1, 3}, Vector3{1, 2, 2}};
    (void)presentation.add(line);
    const Vector3 viewCenter{4, -1, 8};
    const auto vertices = presentedLineVertices(presentation, {viewCenter, 4});
    ASSERT_EQ(vertices.size(), 2U);
    for (const auto& vertex : vertices)
        EXPECT_TRUE(line.contains(Point3{vertex.x(), vertex.y(), vertex.z()}, 1e-11));
    const Vector3 midpoint = (vertices[0] + vertices[1]) / 2.0;
    const auto closest = microsw::geometry::closestPoint(line,
        Point3{viewCenter.x(), viewCenter.y(), viewCenter.z()});
    expectVectorNear(midpoint, {closest.x(), closest.y(), closest.z()}, 1e-12);
    EXPECT_NEAR((vertices[1] - vertices[0]).length(), 24.0, 1e-12);
}

TEST(PresentedLines, MultipleMixedLinesPreserveInsertionOrder)
{
    GeometryPresentation presentation;
    (void)presentation.add(Line3{Point3{0, 1, 0}, Vector3{1, 0, 0}});
    (void)presentation.add(Point3{100, 100, 100});
    (void)presentation.add(Line3{Point3{2, 0, 0}, Vector3{0, 1, 0}});
    (void)presentation.add(Segment3{Point3{}, Point3{1, 1, 1}});
    const auto vertices = presentedLineVertices(presentation, {{}, 1});
    ASSERT_EQ(vertices.size(), 4U);
    expectVectorNear(vertices[0], {-3, 1, 0});
    expectVectorNear(vertices[1], {3, 1, 0});
    expectVectorNear(vertices[2], {2, -3, 0});
    expectVectorNear(vertices[3], {2, 3, 0});
}

TEST(PresentedLines, ScaleChangesLengthProportionally)
{
    GeometryPresentation presentation;
    (void)presentation.add(Line3{Point3{}, Vector3{1, 0, 0}});
    const auto small = presentedLineVertices(presentation, {{}, 2});
    const auto large = presentedLineVertices(presentation, {{}, 5});
    EXPECT_NEAR((small[1] - small[0]).length(), 12, 1e-12);
    EXPECT_NEAR((large[1] - large[0]).length(), 30, 1e-12);
}

TEST(PresentedLines, TranslatedOriginOnSupportProducesSameEndpoints)
{
    GeometryPresentation first, second;
    (void)first.add(Line3{Point3{1, 2, 3}, Vector3{1, 0, 0}});
    (void)second.add(Line3{Point3{101, 2, 3}, Vector3{1, 0, 0}});
    const auto a = presentedLineVertices(first, {{7, 8, 9}, 4});
    const auto b = presentedLineVertices(second, {{7, 8, 9}, 4});
    expectVectorNear(a[0], b[0]);
    expectVectorNear(a[1], b[1]);
}

TEST(PresentedLines, OppositeDirectionProducesSameEndpointSetInReverse)
{
    GeometryPresentation forward, reverse;
    (void)forward.add(Line3{Point3{1, 2, 3}, Vector3{1, 2, 0}});
    (void)reverse.add(Line3{Point3{1, 2, 3}, Vector3{-1, -2, 0}});
    const auto a = presentedLineVertices(forward, {{4, 5, 6}, 3});
    const auto b = presentedLineVertices(reverse, {{4, 5, 6}, 3});
    expectVectorNear(a[0], b[1]);
    expectVectorNear(a[1], b[0]);
}

TEST(PresentedLines, LargeFiniteCoordinatesRemainRepresentable)
{
    GeometryPresentation presentation;
    const Line3 line{Point3{1e150, -1e150, 2e150}, Vector3{0, 1, 0}};
    (void)presentation.add(line);
    const auto vertices = presentedLineVertices(presentation, {{1e150, 0, 2e150}, 10});
    ASSERT_EQ(vertices.size(), 2U);
    EXPECT_TRUE(std::isfinite(vertices[0].x()));
    EXPECT_TRUE(std::isfinite(vertices[0].y()));
    EXPECT_TRUE(std::isfinite(vertices[0].z()));
    EXPECT_EQ(vertices[0].x(), 1e150);
    EXPECT_EQ(vertices[1].z(), 2e150);
}

TEST(PresentedLines, InvalidContextFailsExplicitly)
{
    GeometryPresentation presentation;
    for (double scale : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::quiet_NaN()})
        EXPECT_THROW((void)presentedLineVertices(presentation, {{}, scale}), std::invalid_argument);
    EXPECT_THROW((void)presentedLineVertices(presentation,
        {{std::numeric_limits<double>::infinity(), 0, 0}, 1}), std::invalid_argument);
    EXPECT_THROW((void)presentedLineVertices(presentation,
        {{}, std::numeric_limits<double>::max()}), std::overflow_error);
}
}
