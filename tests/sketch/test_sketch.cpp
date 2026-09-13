#include "core/sketch/Sketch.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
using namespace microsw;
using namespace microsw::sketch;

TEST(SketchEntityId, IsStrongSketchLocalIdentity)
{
    static_assert(!std::is_convertible_v<SketchEntityId, std::uint32_t>);
    EXPECT_FALSE(SketchEntityId{}.isValid());
    EXPECT_EQ(SketchEntityId{}.value(), UINT32_MAX);
    EXPECT_NE(SketchEntityId{0}, SketchEntityId{1});
}

TEST(Sketch, OwnsPlaneAndIndexedTypedEntities)
{
    const SketchPlane plane{geometry::Point3{1, 2, 3}, math::Vector3{0, 1, 0}, math::Vector3{0, 0, 1}};
    Sketch sketch{plane};
    const auto line = sketch.addLine({geometry::Point2{}, geometry::Point2{1, 0}});
    const auto circle = sketch.addCircle({geometry::Point2{2, 3}, 4});
    const auto arc = sketch.addArc({geometry::Point2{}, 2, 0, 1});

    EXPECT_EQ(line.value(), 0u);
    EXPECT_EQ(circle.value(), 1u);
    EXPECT_EQ(arc.value(), 2u);
    EXPECT_EQ(sketch.size(), 3u);
    EXPECT_EQ(sketch.find(line).type(), SketchEntityType::Line);
    EXPECT_EQ(sketch.find(circle).type(), SketchEntityType::Circle);
    EXPECT_EQ(sketch.find(arc).type(), SketchEntityType::Arc);
    EXPECT_EQ(sketch.entityIds(), (std::vector<SketchEntityId>{line, circle, arc}));
    EXPECT_TRUE(geometry::areCoincident(sketch.plane().origin(), plane.origin(), 0));
    EXPECT_THROW((void)sketch.find(SketchEntityId{}), std::out_of_range);
}

TEST(Sketch, CopyAndMovePreserveOwnedValues)
{
    Sketch original;
    const auto id = original.addCircle({geometry::Point2{1, 2}, 3});
    const auto copy = original;
    auto moved = copy;
    EXPECT_EQ(moved.find(id).type(), SketchEntityType::Circle);
    const auto& circle = std::get<SketchCircle>(moved.find(id).geometry()).geometry;
    EXPECT_DOUBLE_EQ(circle.radius(), 3);
}

TEST(Sketch, ReplacementPreservesIdentityAndType)
{
    Sketch sketch;
    const auto line = sketch.addLine({geometry::Point2{}, geometry::Point2{1, 0}});
    const auto circle = sketch.addCircle({geometry::Point2{}, 2});
    const auto arc = sketch.addArc({geometry::Point2{}, 2, 0, 1});

    sketch.replaceLine(line, {geometry::Point2{3, 4}, geometry::Point2{5, 6}});
    sketch.replaceCircle(circle, {geometry::Point2{7, 8}, 9});
    sketch.replaceArc(arc, {geometry::Point2{1, 2}, 3, 4, -1});

    EXPECT_EQ(sketch.find(line).id(), line);
    EXPECT_EQ(sketch.find(circle).id(), circle);
    EXPECT_EQ(sketch.find(arc).id(), arc);
    EXPECT_DOUBLE_EQ(std::get<SketchCircle>(sketch.find(circle).geometry()).geometry.radius(), 9);
    EXPECT_THROW(sketch.replaceCircle(line, {geometry::Point2{}, 3}), std::invalid_argument);
    EXPECT_EQ(sketch.find(line).type(), SketchEntityType::Line);
}

TEST(Sketch, InvalidReplacementValueCannotPartiallyChangeState)
{
    Sketch sketch;
    const auto circle = sketch.addCircle({geometry::Point2{1, 2}, 3});
    EXPECT_THROW(sketch.replaceCircle(circle, {geometry::Point2{9, 9}, 0}), std::invalid_argument);
    const auto& retained = std::get<SketchCircle>(sketch.find(circle).geometry()).geometry;
    EXPECT_DOUBLE_EQ(retained.center().x(), 1);
    EXPECT_DOUBLE_EQ(retained.radius(), 3);
}

TEST(Sketch, RemovalCreatesTombstoneAndIdsAreNeverReused)
{
    Sketch sketch;
    const auto removed = sketch.addLine({geometry::Point2{}, geometry::Point2{1, 0}});
    const auto survivor = sketch.addCircle({geometry::Point2{}, 2});
    sketch.remove(removed);
    EXPECT_FALSE(sketch.contains(removed));
    EXPECT_TRUE(sketch.contains(survivor));
    EXPECT_EQ(sketch.size(), 1u);
    EXPECT_EQ(sketch.entityIds(), (std::vector<SketchEntityId>{survivor}));
    EXPECT_THROW((void)sketch.find(removed), std::out_of_range);
    EXPECT_THROW(sketch.remove(removed), std::out_of_range);

    const auto next = sketch.addArc({geometry::Point2{}, 2, 0, 1});
    EXPECT_EQ(next.value(), 2u);
    EXPECT_NE(next, removed);
}

TEST(Sketch, RectangleAtomicallyCreatesFourIndependentLines)
{
    Sketch sketch;
    const auto ids = sketch.addRectangle(geometry::Point2{1, 2}, geometry::Point2{5, 7});
    EXPECT_EQ(sketch.size(), 4u);
    EXPECT_EQ(ids, (std::array<SketchEntityId, 4>{SketchEntityId{0}, SketchEntityId{1},
                                                  SketchEntityId{2}, SketchEntityId{3}}));

    const std::array<geometry::Point2, 4> corners{
        geometry::Point2{1, 2}, geometry::Point2{5, 2},
        geometry::Point2{5, 7}, geometry::Point2{1, 7}};
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        const auto& line = std::get<SketchLine>(sketch.find(ids[i]).geometry()).geometry;
        EXPECT_TRUE(geometry::areCoincident(line.a(), corners[i], 0));
        EXPECT_TRUE(geometry::areCoincident(line.b(), corners[(i + 1) % 4], 0));
    }

    sketch.replaceLine(ids[0], {geometry::Point2{10, 10}, geometry::Point2{11, 10}});
    const auto& unaffected = std::get<SketchLine>(sketch.find(ids[1]).geometry()).geometry;
    EXPECT_TRUE(geometry::areCoincident(unaffected.a(), corners[1], 0));
}

TEST(Sketch, RectangleRejectsDegeneracyWithoutInsertion)
{
    Sketch sketch;
    const auto existing = sketch.addCircle({geometry::Point2{}, 2});
    EXPECT_THROW((void)sketch.addRectangle(geometry::Point2{}, geometry::Point2{0, 2}), std::invalid_argument);
    EXPECT_THROW((void)sketch.addRectangle(geometry::Point2{}, geometry::Point2{2, 5e-10}), std::invalid_argument);
    EXPECT_EQ(sketch.size(), 1u);
    EXPECT_EQ(sketch.entityIds(), (std::vector<SketchEntityId>{existing}));
}
}
