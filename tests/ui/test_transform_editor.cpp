#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

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
