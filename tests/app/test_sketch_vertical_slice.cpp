#include "app/sketch/SketchToolController.h"
#include "presentation/SketchPresentation.h"
#include "viewer/GeometryPicker.h"

#include <gtest/gtest.h>

#include <numbers>

namespace
{
using namespace microsw;

viewer::PickingContext overheadContext()
{
    viewer::PickingContext context;
    context.camera = viewer::OrbitCamera{{}, 20, 0, std::numbers::pi_v<double> / 2};
    context.width = 800;
    context.height = 600;
    context.mouseX = 400;
    context.mouseY = 300;
    return context;
}

void expectPick(const presentation::SketchPresentation& presentation,
                sketch::SketchEntityId sketchId, const geometry::Point3& worldPoint)
{
    auto context = overheadContext();
    const auto screen = viewer::projectWorldToScreen(worldPoint, context);
    ASSERT_TRUE(screen);
    context.mouseX = screen->x;
    context.mouseY = screen->y;
    const auto hit = viewer::pickGeometry(presentation.geometry(), context);
    ASSERT_TRUE(hit);
    EXPECT_EQ(presentation.sketchId(hit->id), sketchId);
}

TEST(SketchVerticalSlice, CreatePickEditAndDeleteRemainCoherent)
{
    sketch::Sketch model;
    SketchToolController tools{model};
    tools.setTool(SketchTool::Line);
    (void)tools.click({-6, -4});
    const auto line = tools.click({-2, -4})[0];
    tools.setTool(SketchTool::Rectangle);
    (void)tools.click({-1, -1});
    const auto rectangle = tools.click({1, 1});
    tools.setTool(SketchTool::Circle);
    (void)tools.click({4, 0});
    const auto circle = tools.click({5, 0})[0];
    tools.setTool(SketchTool::Arc);
    (void)tools.click({0, 4});
    (void)tools.click({1, 4});
    const auto arc = tools.click({0, 5})[0];

    presentation::SketchPresentation presentation;
    presentation.regenerate(model);
    EXPECT_EQ(model.size(), 7u);
    EXPECT_EQ(presentation.geometry().size(), 7u);
    expectPick(presentation, line, {-4, -4, 0});
    expectPick(presentation, circle, {5, 0, 0});
    expectPick(presentation, arc, {0, 5, 0});

    const auto circleVisual = presentation.visualId(circle);
    model.replaceCircle(circle, {{6, 0}, 2});
    presentation.regenerate(model);
    EXPECT_EQ(presentation.visualId(circle), circleVisual);
    expectPick(presentation, circle, {8, 0, 0});

    model.remove(circle);
    presentation.regenerate(model);
    EXPECT_FALSE(presentation.visualId(circle));
    EXPECT_FALSE(presentation.sketchId(*circleVisual));
    EXPECT_EQ(model.size(), 6u);
    EXPECT_EQ(rectangle.size(), 4u);
}
}
