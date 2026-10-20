#include "app/sketch/SketchToolController.h"

#include <gtest/gtest.h>

#include <numbers>
#include <limits>

namespace
{
using namespace microsw;

TEST(SketchTools, LineAndRectangleUseRequiredClicks)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    tools.setTool(SketchTool::Line);
    EXPECT_TRUE(tools.click({1, 2}).empty());
    const auto line = tools.click({3, 4});
    ASSERT_EQ(line.size(), 1u);
    EXPECT_EQ(model.find(line[0]).type(), sketch::SketchEntityType::Line);

    tools.setTool(SketchTool::Rectangle);
    EXPECT_TRUE(tools.click({0, 0}).empty());
    const auto rectangle = tools.click({5, 6});
    EXPECT_EQ(rectangle.size(), 4u);
    EXPECT_EQ(model.size(), 5u);
}

TEST(SketchTools, CircleUsesCenterAndRadiusPoint)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    tools.setTool(SketchTool::Circle);
    (void)tools.click({1, 2});
    const auto result = tools.click({4, 6});
    const auto& circle = std::get<sketch::SketchCircle>(model.find(result[0]).geometry()).geometry;
    EXPECT_DOUBLE_EQ(circle.radius(), 5);
    EXPECT_TRUE(geometry::areCoincident(circle.center(), geometry::Point2{1, 2}, 0));
}

TEST(SketchTools, ArcDerivesDeterministicPositiveCounterClockwiseSweep)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    tools.setTool(SketchTool::Arc);
    (void)tools.click({0, 0});
    (void)tools.click({1, 0});
    const auto result = tools.click({0, -1});
    const auto& arc = std::get<sketch::SketchArc>(model.find(result[0]).geometry()).geometry;
    EXPECT_DOUBLE_EQ(arc.startAngle(), 0);
    EXPECT_NEAR(arc.sweepAngle(), 1.5 * std::numbers::pi, 1e-12);
}

TEST(SketchTools, InvalidCompletionClearsPendingStateAndCreatesNothing)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    tools.setTool(SketchTool::Circle);
    (void)tools.click({1, 1});
    EXPECT_THROW((void)tools.click({1, 1}), std::invalid_argument);
    EXPECT_EQ(tools.pendingPointCount(), 0u);
    EXPECT_EQ(model.size(), 0u);
    tools.setTool(SketchTool::Line);
    (void)tools.click({1, 2});
    tools.setTool(SketchTool::Rectangle);
    EXPECT_EQ(tools.pendingPointCount(), 0u);
}

TEST(SketchTools, NumericRectangleUsesExactCenterAndDimensions)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    const auto lines = tools.addCenteredRectangle({16, 0}, 18, 14);
    ASSERT_EQ(model.size(), 4u);
    const auto& bottom = std::get<sketch::SketchLine>(model.find(lines[0]).geometry()).geometry;
    const auto& right = std::get<sketch::SketchLine>(model.find(lines[1]).geometry()).geometry;
    EXPECT_DOUBLE_EQ(bottom.a().x(), 7);
    EXPECT_DOUBLE_EQ(bottom.a().y(), -7);
    EXPECT_DOUBLE_EQ(bottom.b().x(), 25);
    EXPECT_DOUBLE_EQ(bottom.b().y(), -7);
    EXPECT_DOUBLE_EQ(right.b().x(), 25);
    EXPECT_DOUBLE_EQ(right.b().y(), 7);
}

TEST(SketchTools, InvalidNumericRectangleLeavesSketchUnchanged)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    EXPECT_THROW((void)tools.addCenteredRectangle({0, 0}, 0, 14), std::invalid_argument);
    EXPECT_THROW((void)tools.addCenteredRectangle({0, 0}, 18,
        std::numeric_limits<double>::infinity()), std::invalid_argument);
    EXPECT_EQ(model.size(), 0u);
}
}
