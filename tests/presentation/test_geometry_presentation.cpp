#include "presentation/GeometryPresentation.h"

#include <gtest/gtest.h>

#include <variant>
#include <limits>
#include <stdexcept>

namespace
{
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::areCoincident;
using microsw::math::Vector3;
using microsw::presentation::GeometryPresentation;

TEST(GeometryPresentation, TransformUpdateIsAtomicAndPreservesCollection)
{
    GeometryPresentation presentation;
    const auto id = presentation.add(Point3{2, 0, 0});
    const auto other = presentation.add(Point3{3, 0, 0});
    const microsw::math::Transform3 transform{Vector3{1, 2, 3}, Vector3{0.1, 0.2, 0.3}, Vector3{2, 3, 4}};
    EXPECT_TRUE(presentation.setTransform(id, transform));
    EXPECT_TRUE(microsw::math::almostEqual(presentation.find(id)->transform().matrix(), transform.matrix()));
    EXPECT_FALSE(presentation.setTransform(microsw::presentation::VisualEntityId{999}, transform));
    EXPECT_THROW(presentation.setTransform(id, microsw::math::Transform3(
        Vector3{}, Vector3{}, Vector3{std::numeric_limits<double>::max(), 1, 1})), std::overflow_error);
    EXPECT_TRUE(microsw::math::almostEqual(presentation.find(id)->transform().matrix(), transform.matrix()));
    EXPECT_TRUE(areCoincident(std::get<Point3>(presentation.find(id)->geometry()), Point3{2, 0, 0}, 0));
    EXPECT_TRUE(microsw::math::almostEqual(presentation.find(other)->transform().matrix(), microsw::math::Matrix4::identity()));
    EXPECT_EQ(presentation.entities()[0].id(), id);
    EXPECT_EQ(presentation.entities()[1].id(), other);
}

TEST(GeometryPresentation, DefaultsToEmptyReadOnlyCollection)
{
    const GeometryPresentation presentation;
    EXPECT_TRUE(presentation.empty());
    EXPECT_EQ(presentation.size(), 0U);
    EXPECT_TRUE(presentation.entities().empty());
}

TEST(GeometryPresentation, AddsSupportedPrimitivesWithUniqueIdsInInsertionOrder)
{
    GeometryPresentation presentation;
    const Point3 point{1, 2, 3};
    const Segment3 segment{Point3{4, 5, 6}, Point3{7, 8, 9}};
    const Line3 line{Point3{-1, -2, -3}, Vector3{0, 2, 0}};
    const auto pointId = presentation.add(point);
    const auto segmentId = presentation.add(segment);
    const auto lineId = presentation.add(line);

    ASSERT_EQ(presentation.size(), 3U);
    EXPECT_FALSE(presentation.empty());
    EXPECT_NE(pointId, segmentId);
    EXPECT_NE(segmentId, lineId);
    EXPECT_EQ(presentation.entities()[0].id(), pointId);
    EXPECT_EQ(presentation.entities()[1].id(), segmentId);
    EXPECT_EQ(presentation.entities()[2].id(), lineId);
    EXPECT_TRUE(std::holds_alternative<Point3>(presentation.entities()[0].geometry()));
    EXPECT_TRUE(std::holds_alternative<Segment3>(presentation.entities()[1].geometry()));
    EXPECT_TRUE(std::holds_alternative<Line3>(presentation.entities()[2].geometry()));
    for (const auto& entity : presentation.entities())
    {
        EXPECT_EQ(presentation.find(entity.id()), &entity);
        EXPECT_TRUE(microsw::math::almostEqual(entity.transform().translation(), Vector3{}));
        EXPECT_TRUE(microsw::math::almostEqual(entity.transform().rotation(), Vector3{}));
        EXPECT_TRUE(microsw::math::almostEqual(entity.transform().scale(), Vector3{1, 1, 1}));
        EXPECT_TRUE(microsw::math::almostEqual(
            entity.transform().matrix(), microsw::math::Matrix4::identity()));
    }
}

TEST(GeometryPresentation, OwnsCopiedGeometryValues)
{
    GeometryPresentation presentation;
    Point3 point{1, 2, 3};
    Segment3 segment{Point3{4, 5, 6}, Point3{7, 8, 9}};
    Line3 line{Point3{-1, -2, -3}, Vector3{0, 2, 0}};
    (void)presentation.add(point);
    (void)presentation.add(segment);
    (void)presentation.add(line);
    point = Point3{};
    segment = Segment3{};
    line = Line3{Point3{}, Vector3{1, 0, 0}};

    EXPECT_TRUE(areCoincident(std::get<Point3>(presentation.entities()[0].geometry()), Point3{1, 2, 3}, 0));
    EXPECT_TRUE(areCoincident(std::get<Segment3>(presentation.entities()[1].geometry()).a(), Point3{4, 5, 6}, 0));
    EXPECT_TRUE(areCoincident(std::get<Line3>(presentation.entities()[2].geometry()).origin(), Point3{-1, -2, -3}, 0));
}

TEST(GeometryPresentation, FindsExistingIdsAndReturnsNullForUnknownId)
{
    GeometryPresentation presentation;
    const auto firstId = presentation.add(Point3{1, 2, 3});
    const auto secondId = presentation.add(Point3{4, 5, 6});
    ASSERT_NE(presentation.find(firstId), nullptr);
    EXPECT_EQ(presentation.find(firstId)->id(), firstId);
    ASSERT_NE(presentation.find(secondId), nullptr);
    EXPECT_EQ(presentation.find(secondId)->id(), secondId);
    EXPECT_EQ(presentation.find(microsw::presentation::VisualEntityId{999}), nullptr);
}
}
