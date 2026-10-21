#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>
#include <array>

TEST(TransformEditor, DrawsEverySelectedPrimitiveAndHandlesDeselection)
{
    microsw::ApplicationWindow window{640, 480, "Transform editor test"};
    glfwHideWindow(static_cast<GLFWwindow*>(window.nativeHandle()));
    microsw::OpenGLContext context{window};
    microsw::ImGuiLayer ui{window, context};
    microsw::ApplicationShell shell{window};
    using namespace microsw;
    const presentation::PresentedGeometry geometries[]{geometry::Point3{1, 2, 3},
        geometry::Segment3{geometry::Point3{}, geometry::Point3{1, 2, 3}},
        geometry::Line3{geometry::Point3{}, math::Vector3{1, 0, 0}}};
    for (const auto& geometry : geometries)
    {
        presentation::VisualEntity selected{presentation::VisualEntityId{1}, geometry};
        selected.setTransform(math::Transform3{math::Vector3{1, 2, 3},
            math::Vector3{0.1, 0.2, 0.3}, math::Vector3{2, 3, 4}});
        ui.beginFrame();
        shell.draw(ProjectionMode::Perspective, &selected);
        EXPECT_FALSE(shell.transformRequest());
        ui.endFrame();
    }
    ui.beginFrame();
    shell.draw(ProjectionMode::Orthographic);
    EXPECT_FALSE(shell.transformRequest());
    ui.endFrame();
}

TEST(SketchEditor, DrawsEverySketchEntityTypeAndToolState)
{
    microsw::ApplicationWindow window{640, 480, "Sketch editor test"};
    glfwHideWindow(static_cast<GLFWwindow*>(window.nativeHandle()));
    microsw::OpenGLContext context{window};
    microsw::ImGuiLayer ui{window, context};
    microsw::ApplicationShell shell{window};
    microsw::sketch::Sketch model;
    const auto ids = std::array{
        model.addLine({{0, 0}, {1, 0}}),
        model.addCircle({{0, 0}, 2}),
        model.addArc({{0, 0}, 2, 0, 1})};
    for (const auto id : ids)
    {
        ui.beginFrame();
        shell.drawSketch(microsw::ProjectionMode::Perspective,
                         microsw::SketchTool::Select, model, &model.find(id));
        EXPECT_FALSE(shell.sketchGeometryRequest());
        EXPECT_FALSE(shell.deleteSketchRequest());
        ui.endFrame();
    }
    ui.beginFrame();
    shell.drawSketch(microsw::ProjectionMode::Orthographic,
                     microsw::SketchTool::Circle, model);
    EXPECT_FALSE(shell.sketchToolRequest());
    ui.endFrame();
}

TEST(SketchEditor, DrawsSelectedDimensionWithoutSelectedGeometry)
{
    microsw::ApplicationWindow window{900, 700, "Sketch dimension inspector test"};
    glfwHideWindow(static_cast<GLFWwindow*>(window.nativeHandle()));
    microsw::OpenGLContext context{window};
    microsw::ImGuiLayer ui{window, context};
    microsw::ApplicationShell shell{window};
    microsw::sketch::Sketch sketch;
    const auto line = sketch.addLine({{0, 0}, {30, 0}});
    const auto dimension = sketch.addConstraint(microsw::sketch::LineLength{
        {line, microsw::sketch::SubElementKind::LineBody}, 30});
    microsw::features::FeatureModel features;
    const auto sketchFeature = features.addSketch(sketch);
    ASSERT_EQ(features.rebuild().status, microsw::features::RebuildStatus::Success);
    microsw::interaction::EditorContext editor;
    editor.activateSketch(sketchFeature);
    editor.select(microsw::interaction::LogicalTarget{dimension});

    ui.beginFrame();
    shell.drawSketch(microsw::ProjectionMode::Orthographic,
        microsw::SketchTool::Select, sketch, nullptr, false, &features, &editor);
    EXPECT_FALSE(shell.sketchGeometryRequest());
    EXPECT_FALSE(shell.deleteSketchRequest());
    ui.endFrame();
}

TEST(FeatureTree, DrawsHistoryStatesDependenciesAndParameters)
{
    microsw::ApplicationWindow window{900, 700, "Feature tree test"};
    glfwHideWindow(static_cast<GLFWwindow*>(window.nativeHandle()));
    microsw::OpenGLContext context{window};
    microsw::ImGuiLayer ui{window, context};
    microsw::ApplicationShell shell{window};
    microsw::features::FeatureModel features;
    microsw::sketch::Sketch a;
    (void)a.addRectangle({0, 0}, {4, 4});
    microsw::sketch::Sketch b{{{2, 2, 1}, {1, 0, 0}, {0, 1, 0}}};
    (void)b.addRectangle({0, 0}, {4, 4});
    const auto ea = features.addExtrude(features.addSketch(a), 4);
    const auto eb = features.addExtrude(features.addSketch(b), 4);
    (void)features.addBoolean(ea, eb, microsw::modeling::BooleanOperation::Intersection);
    ASSERT_EQ(features.rebuild().status, microsw::features::RebuildStatus::Success);
    microsw::sketch::Sketch editorSketch;
    ui.beginFrame();
    shell.drawSketch(microsw::ProjectionMode::Perspective,
        microsw::SketchTool::Select, editorSketch, nullptr, true, &features);
    EXPECT_FALSE(shell.rebuildFeaturesRequest());
    EXPECT_FALSE(shell.featureDistanceRequest());
    EXPECT_FALSE(shell.featureBooleanRequest());
    ui.endFrame();
}

TEST(CadWorkspaceLayout, KeepsGraphicsAreaUsableAtDesktopSizes)
{
    microsw::ApplicationWindow window{1366, 768, "CAD workspace layout test"};
    glfwHideWindow(static_cast<GLFWwindow*>(window.nativeHandle()));
    microsw::OpenGLContext context{window};
    microsw::ImGuiLayer ui{window, context};
    microsw::ApplicationShell shell{window};
    microsw::features::FeatureModel features;
    microsw::interaction::EditorContext editor;
    microsw::sketch::Sketch sketch;

    for (const auto [width, height] : std::array<std::array<int, 2>, 2>{
             std::array{1366, 768}, std::array{1920, 1080}})
    {
        glfwSetWindowSize(static_cast<GLFWwindow*>(window.nativeHandle()), width, height);
        glfwPollEvents();
        ui.beginFrame();
        shell.drawSketch(microsw::ProjectionMode::Perspective,
            microsw::SketchTool::Select, sketch, nullptr, false, &features, &editor);
        const auto& graphics = shell.workspaceRect();
        EXPECT_GE(graphics.width, width * 0.50) << width << "x" << height;
        EXPECT_GE(graphics.height, height * 0.60) << width << "x" << height;
        EXPECT_GT(graphics.x, 0);
        EXPECT_LE(graphics.x + graphics.width, graphics.displayWidth);
        EXPECT_LE(graphics.y + graphics.height, graphics.displayHeight);
        ui.endFrame();
    }
}
