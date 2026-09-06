#include "viewer/PresentedPoints.h"

#include "presentation/GeometryPresentation.h"

#include <gtest/gtest.h>

namespace
{
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::math::Vector3;
using microsw::presentation::GeometryPresentation;
using microsw::viewer::presentedPointVertices;

TEST(PresentedPoints, ConvertsPointValuesInInsertionOrderWithoutUsingIds)
{
    GeometryPresentation presentation;
    const auto firstId = presentation.add(Point3{1, 2, 3});
    const auto secondId = presentation.add(Point3{-4, 5, -6});
    const auto vertices = presentedPointVertices(presentation);
    ASSERT_EQ(vertices.size(), 2U);
    EXPECT_EQ(vertices[0].x(), 1);
    EXPECT_EQ(vertices[0].y(), 2);
    EXPECT_EQ(vertices[0].z(), 3);
    EXPECT_EQ(vertices[1].x(), -4);
    EXPECT_EQ(vertices[1].y(), 5);
    EXPECT_EQ(vertices[1].z(), -6);
    EXPECT_NE(firstId, secondId);
}

TEST(PresentedPoints, IgnoresSegmentAndLineAndLeavesGeometryUnchanged)
{
    GeometryPresentation presentation;
    const Segment3 segment{Point3{1, 2, 3}, Point3{4, 5, 6}};
    const Line3 line{Point3{7, 8, 9}, Vector3{1, 0, 0}};
    (void)presentation.add(segment);
    (void)presentation.add(Point3{-1, -2, -3});
    (void)presentation.add(line);
    const auto vertices = presentedPointVertices(presentation);
    ASSERT_EQ(vertices.size(), 1U);
    EXPECT_EQ(vertices.front().x(), -1);
    EXPECT_EQ(segment.a().x(), 1);
    EXPECT_EQ(line.origin().x(), 7);
}
}
