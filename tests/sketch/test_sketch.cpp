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
}
