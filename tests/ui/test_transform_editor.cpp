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
