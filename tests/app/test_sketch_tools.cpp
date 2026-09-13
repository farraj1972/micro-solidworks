#include "app/sketch/SketchToolController.h"

#include <gtest/gtest.h>

#include <numbers>

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
}
