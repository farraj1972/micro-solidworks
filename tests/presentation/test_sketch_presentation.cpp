#include "presentation/SketchPresentation.h"
#include "viewer/PresentedSegments.h"

#include <gtest/gtest.h>

namespace
{
using namespace microsw;

TEST(SketchPresentation, DerivesWorldGeometryAndStableBidirectionalIdentity)
{
    sketch::Sketch model{sketch::SketchPlane{geometry::Point3{1, 2, 3},
        math::Vector3{1, 0, 0}, math::Vector3{0, 0, 1}}};
    const auto line = model.addLine({geometry::Point2{}, geometry::Point2{2, 4}});
    const auto circle = model.addCircle({geometry::Point2{3, 4}, 2});
    const auto arc = model.addArc({geometry::Point2{-1, 2}, 3, 0, 1.5});

    presentation::SketchPresentation presented;
    presented.regenerate(model);
    ASSERT_EQ(presented.geometry().size(), 3u);
    const auto lineVisual = presented.visualId(line);
    const auto circleVisual = presented.visualId(circle);
    const auto arcVisual = presented.visualId(arc);
    ASSERT_TRUE(lineVisual && circleVisual && arcVisual);
    EXPECT_EQ(presented.sketchId(*lineVisual), line);
    EXPECT_EQ(presented.sketchId(*circleVisual), circle);

    const auto& segment = std::get<geometry::Segment3>(
        presented.geometry().find(*lineVisual)->geometry());
    EXPECT_TRUE(geometry::areCoincident(segment.a(), geometry::Point3{1, 2, 3}, 0));
    EXPECT_TRUE(geometry::areCoincident(segment.b(), geometry::Point3{3, 2, 7}, 0));
    EXPECT_TRUE(std::holds_alternative<presentation::Polyline3>(
        presented.geometry().find(*circleVisual)->geometry()));

    model.replaceCircle(circle, {geometry::Point2{8, 9}, 4});
    presented.regenerate(model);
    EXPECT_EQ(presented.visualId(line), lineVisual);
    EXPECT_EQ(presented.visualId(circle), circleVisual);
    EXPECT_EQ(presented.visualId(arc), arcVisual);

    model.remove(circle);
    presented.regenerate(model);
    EXPECT_FALSE(presented.visualId(circle));
    EXPECT_EQ(presented.geometry().find(*circleVisual), nullptr);
    EXPECT_EQ(presented.geometry().size(), 2u);
}

TEST(SketchPresentation, CurveRenderVerticesUseTheDerivedPolyline)
{
    sketch::Sketch model;
    const auto circle = model.addCircle({geometry::Point2{}, 2});
    presentation::SketchPresentation presented;
    presented.regenerate(model);
    const auto visual = *presented.visualId(circle);
    const auto& polyline = std::get<presentation::Polyline3>(presented.geometry().find(visual)->geometry());
    const auto vertices = viewer::presentedSegmentVertices(presented.geometry());
    ASSERT_EQ(vertices.size(), (polyline.points().size() - 1) * 2);
    EXPECT_DOUBLE_EQ(vertices.front().x(), polyline.points().front().x());
    EXPECT_DOUBLE_EQ(vertices.back().y(), polyline.points().back().y());
}
}
