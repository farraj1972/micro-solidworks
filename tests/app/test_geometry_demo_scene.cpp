#include "app/demo/GeometryDemoScene.h"

#include "viewer/PresentedLines.h"
#include "viewer/PresentedPoints.h"
#include "viewer/PresentedSegments.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <set>
#include <variant>

namespace
{
using microsw::demo::createGeometryDemoScene;
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::math::Vector3;

void expectPointNear(const Point3& actual, const Point3& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), 1e-12);
    EXPECT_NEAR(actual.y(), expected.y(), 1e-12);
    EXPECT_NEAR(actual.z(), expected.z(), 1e-12);
}

void expectVectorNear(const Vector3& actual, const Vector3& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), 1e-12);
    EXPECT_NEAR(actual.y(), expected.y(), 1e-12);
    EXPECT_NEAR(actual.z(), expected.z(), 1e-12);
}

TEST(GeometryDemoScene, ContainsExactlyThreePointsThreeSegmentsAndTwoLines)
{
    const auto presentation = createGeometryDemoScene();
    EXPECT_FALSE(presentation.empty());
    ASSERT_EQ(presentation.size(), 8U);
    std::size_t points = 0, segments = 0, lines = 0;
    for (const auto& entity : presentation.entities())
    {
        if (std::holds_alternative<Point3>(entity.geometry()))
            ++points;
        else if (std::holds_alternative<Segment3>(entity.geometry()))
            ++segments;
        else if (std::holds_alternative<Line3>(entity.geometry()))
            ++lines;
        else
            ADD_FAILURE() << "Unsupported demo geometry";
    }
    EXPECT_EQ(points, 3U);
    EXPECT_EQ(segments, 3U);
    EXPECT_EQ(lines, 2U);
}

TEST(GeometryDemoScene, PreservesExpectedGeometryValuesInPointSegmentLineOrder)
{
    const auto presentation = createGeometryDemoScene();
    const auto& entities = presentation.entities();
    ASSERT_EQ(entities.size(), 8U);
    const std::array points{
        Point3{0, 0, 0}, Point3{2, 2, 1}, Point3{-2, 1, 2}};
    const std::array segments{
        Segment3{Point3{-3, -2, 0.5}, Point3{3, -2, 0.5}},
        Segment3{Point3{-3, 2, 0.5}, Point3{-1, 4, 2.5}},
        Segment3{Point3{3, 1, 0.5}, Point3{3, 2, 3.5}}};
    const std::array origins{Point3{-2, 3, 1}, Point3{2, -1, 2}};
    const std::array directions{
        Vector3{1, 1, 0.25}.normalized(), Vector3{-0.5, 1, 1.5}.normalized()};

    for (std::size_t i = 0; i < points.size(); ++i)
    {
        SCOPED_TRACE(i);
        const auto* point = std::get_if<Point3>(&entities[i].geometry());
        ASSERT_NE(point, nullptr);
        expectPointNear(*point, points[i]);
    }
    for (std::size_t i = 0; i < segments.size(); ++i)
    {
        SCOPED_TRACE(i);
        const auto* segment = std::get_if<Segment3>(&entities[i + 3].geometry());
        ASSERT_NE(segment, nullptr);
        expectPointNear(segment->a(), segments[i].a());
        expectPointNear(segment->b(), segments[i].b());
    }
    for (std::size_t i = 0; i < origins.size(); ++i)
    {
        SCOPED_TRACE(i);
        const auto* line = std::get_if<Line3>(&entities[i + 6].geometry());
        ASSERT_NE(line, nullptr);
        expectPointNear(line->origin(), origins[i]);
        expectVectorNear(line->direction(), directions[i]);
    }
}

TEST(GeometryDemoScene, HasValidUniqueIdsWithinEachCollection)
{
    const auto presentation = createGeometryDemoScene();
    std::set<std::uint64_t> ids;
    for (const auto& entity : presentation.entities())
    {
        EXPECT_NE(entity.id().value(), 0U);
        EXPECT_TRUE(ids.insert(entity.id().value()).second);
    }
    EXPECT_EQ(ids.size(), presentation.size());
}

TEST(GeometryDemoScene, RepeatedConstructionPreservesContentAndOrdering)
{
    const auto first = createGeometryDemoScene();
    const auto second = createGeometryDemoScene();
    ASSERT_EQ(first.size(), 8U);
    ASSERT_EQ(first.size(), second.size());
    // Each collection owns its generator. Compare content, without requiring
    // globally distinct IDs or assigning persistent meaning to their values.
    for (std::size_t i = 0; i < first.size(); ++i)
    {
        SCOPED_TRACE(i);
        const auto& a = first.entities()[i].geometry();
        const auto& b = second.entities()[i].geometry();
        ASSERT_EQ(a.index(), b.index());
        if (const auto* point = std::get_if<Point3>(&a))
            expectPointNear(*point, std::get<Point3>(b));
        else if (const auto* segment = std::get_if<Segment3>(&a))
        {
            expectPointNear(segment->a(), std::get<Segment3>(b).a());
            expectPointNear(segment->b(), std::get<Segment3>(b).b());
        }
        else if (const auto* line = std::get_if<Line3>(&a))
        {
            expectPointNear(line->origin(), std::get<Line3>(b).origin());
            expectVectorNear(line->direction(), std::get<Line3>(b).direction());
        }
        else
            ADD_FAILURE() << "Unsupported demo geometry";
    }
}

TEST(GeometryDemoScene, ExistingAdaptersConsumeTheRealDemo)
{
    const auto presentation = createGeometryDemoScene();
    EXPECT_EQ(microsw::viewer::presentedPointVertices(presentation).size(), 3U);
    EXPECT_EQ(microsw::viewer::presentedSegmentVertices(presentation).size(), 6U);
    const std::array contexts{
        microsw::viewer::LinePresentationContext{{}, 10},
        microsw::viewer::LinePresentationContext{{4, -3, 2}, 2},
        microsw::viewer::LinePresentationContext{{-10, 5, -2}, 25}};
    for (const auto& context : contexts)
        EXPECT_EQ(microsw::viewer::presentedLineVertices(presentation, context).size(), 4U);
}
}
