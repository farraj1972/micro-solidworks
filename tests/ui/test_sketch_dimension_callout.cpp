#include "ui/SketchDimensionCallout.h"

#include <gtest/gtest.h>

namespace
{
using microsw::interaction::ScreenPoint;
using microsw::sketch::Sketch;
using microsw::sketch::SketchElementRef;
using microsw::sketch::SubElementKind;

std::optional<ScreenPoint> identityProject(const microsw::geometry::Point2& point)
{
    return ScreenPoint{point.x(), point.y()};
}

TEST(SketchDimensionCallout, DerivesOnlyDrivingDimensionsFromStableConstraintIds)
{
    Sketch sketch;
    const auto line = sketch.addLine({{0, 0}, {40, 0}});
    const auto circle = sketch.addCircle({{10, 20}, 6});
    const auto length = sketch.addConstraint(
        microsw::sketch::LineLength{{line, SubElementKind::LineBody}, 40});
    const auto radius = sketch.addConstraint(
        microsw::sketch::CircleRadius{{circle, SubElementKind::CircleRadius}, 6});
    (void)sketch.addConstraint(microsw::sketch::Horizontal{{line, SubElementKind::LineBody}});
    const auto originalCount = sketch.constraintCount();

    const auto callouts = microsw::ui::deriveSketchDimensionCallouts(sketch, identityProject);
    ASSERT_EQ(callouts.size(), 2u);
    EXPECT_EQ(callouts[0].constraint, length);
    EXPECT_EQ(callouts[0].entity, line);
    EXPECT_EQ(callouts[0].text, "L 40 mm");
    EXPECT_EQ(callouts[1].constraint, radius);
    EXPECT_EQ(callouts[1].entity, circle);
    EXPECT_EQ(callouts[1].text, "R 6 mm");
    EXPECT_EQ(sketch.constraintCount(), originalCount);
}

TEST(SketchDimensionCallout, UsesProjectorForAnySketchPlane)
{
    Sketch sketch{{{1, 2, 3}, {0, 1, 0}, {0, 0, 1}}};
    const auto line = sketch.addLine({{1, 2}, {5, 2}});
    (void)sketch.addConstraint(microsw::sketch::LineLength{{line, SubElementKind::LineBody}, 4});
    const auto callouts = microsw::ui::deriveSketchDimensionCallouts(sketch,
        [&](const microsw::geometry::Point2& local) -> std::optional<ScreenPoint> {
            const auto world = sketch.plane().toWorld(local);
            return ScreenPoint{world.y() * 2, world.z() * 3};
        });
    ASSERT_EQ(callouts.size(), 1u);
    EXPECT_DOUBLE_EQ(callouts[0].first.x, 6);
    EXPECT_DOUBLE_EQ(callouts[0].second.x, 14);
    EXPECT_DOUBLE_EQ(callouts[0].first.y, 15);
}

TEST(SketchDimensionCallout, SupportsCoordinateDimensionsAndClippedProjection)
{
    Sketch sketch;
    const auto line = sketch.addLine({{2, 3}, {8, 9}});
    const SketchElementRef start{line, SubElementKind::LineStart};
    const SketchElementRef end{line, SubElementKind::LineEnd};
    (void)sketch.addConstraint(microsw::sketch::HorizontalDistance{start, end, 6});
    (void)sketch.addConstraint(microsw::sketch::VerticalDistance{start, end, 6});
    const auto callouts = microsw::ui::deriveSketchDimensionCallouts(sketch, identityProject);
    ASSERT_EQ(callouts.size(), 2u);
    EXPECT_EQ(callouts[0].text, "X 6 mm");
    EXPECT_EQ(callouts[1].text, "Y 6 mm");
    const auto clipped = microsw::ui::deriveSketchDimensionCallouts(sketch,
        [](const microsw::geometry::Point2&) -> std::optional<ScreenPoint> { return std::nullopt; });
    EXPECT_TRUE(clipped.empty());
}
}
