#include "viewer/PresentedSegments.h"

#include "presentation/GeometryPresentation.h"

#include <gtest/gtest.h>

namespace
{
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::math::Vector3;
using microsw::presentation::GeometryPresentation;
using microsw::viewer::presentedSegmentVertices;

void expectVertex(const Vector3& vertex, double x, double y, double z)
{
    EXPECT_EQ(vertex.x(), x);
    EXPECT_EQ(vertex.y(), y);
    EXPECT_EQ(vertex.z(), z);
}

TEST(PresentedSegments, EmptyPointOnlyAndLineOnlyPresentationsProduceNoVertices)
{
    GeometryPresentation empty;
    EXPECT_TRUE(presentedSegmentVertices(empty).empty());
    GeometryPresentation points;
    (void)points.add(Point3{1, 2, 3});
    EXPECT_TRUE(presentedSegmentVertices(points).empty());
    GeometryPresentation lines;
    (void)lines.add(Line3{Point3{}, Vector3{1, 0, 0}});
    EXPECT_TRUE(presentedSegmentVertices(lines).empty());
}

TEST(PresentedSegments, SingleSegmentConvertsAThenBExactly)
{
    GeometryPresentation presentation;
    const auto id = presentation.add(Segment3{Point3{-1, 2, -3}, Point3{4, -5, 6}});
    const auto vertices = presentedSegmentVertices(presentation);
    ASSERT_EQ(vertices.size(), 2U);
    expectVertex(vertices[0], -1, 2, -3);
    expectVertex(vertices[1], 4, -5, 6);
    EXPECT_EQ(presentation.find(id)->id(), id);
}

TEST(PresentedSegments, MultipleMixedEntitiesPreserveSegmentInsertionOrder)
{
    GeometryPresentation presentation;
    const auto firstId = presentation.add(Segment3{Point3{1, 2, 3}, Point3{4, 5, 6}});
    (void)presentation.add(Point3{100, 100, 100});
    const auto secondId = presentation.add(Segment3{Point3{-7, -8, -9}, Point3{10, 11, 12}});
    (void)presentation.add(Line3{Point3{}, Vector3{0, 1, 0}});
    const auto vertices = presentedSegmentVertices(presentation);
    ASSERT_EQ(vertices.size(), 4U);
    expectVertex(vertices[0], 1, 2, 3);
    expectVertex(vertices[1], 4, 5, 6);
    expectVertex(vertices[2], -7, -8, -9);
    expectVertex(vertices[3], 10, 11, 12);
    EXPECT_NE(firstId, secondId);
}

TEST(PresentedSegments, DegenerateSegmentProducesTwoIdenticalVertices)
{
    GeometryPresentation presentation;
    (void)presentation.add(Segment3{Point3{-2, 3, 4}, Point3{-2, 3, 4}});
    const auto vertices = presentedSegmentVertices(presentation);
    ASSERT_EQ(vertices.size(), 2U);
    expectVertex(vertices[0], -2, 3, 4);
    expectVertex(vertices[1], -2, 3, 4);
}

TEST(PresentedSegments, ConversionDoesNotMutateGeometryOrUseIdentityAsVertexData)
{
    GeometryPresentation presentation;
    const Segment3 original{Point3{21, 22, 23}, Point3{24, 25, 26}};
    const auto id = presentation.add(original);
    const auto vertices = presentedSegmentVertices(presentation);
    ASSERT_EQ(vertices.size(), 2U);
    expectVertex(vertices[0], 21, 22, 23);
    expectVertex(vertices[1], 24, 25, 26);
    EXPECT_EQ(original.a().x(), 21);
    EXPECT_EQ(original.b().z(), 26);
    EXPECT_NE(static_cast<double>(id.value()), vertices[0].x());
}
}
