#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "presentation/GeometryPresentation.h"
#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "viewer/WorkspaceViewport.h"

#include <exception>

int main()
{
    microsw::Logger::info("Application starting");

    try
    {
        {
            microsw::ApplicationWindow window{1280, 720, "Micro SolidWorks"};
            microsw::OpenGLContext openGLContext{window};
            microsw::ImGuiLayer ui{window, openGLContext};
            microsw::ApplicationShell shell{window};
            microsw::presentation::GeometryPresentation presentation;
            (void)presentation.add(microsw::geometry::Point3{0, 0, 0});
            (void)presentation.add(microsw::geometry::Point3{2, 2, 1});
            (void)presentation.add(microsw::geometry::Point3{-2, 1, 2});
            microsw::viewer::WorkspaceViewport workspace{presentation};

            while (!window.shouldClose())
            {
                window.pollEvents();
                // Whole-frame background before UI composition; the 3D pass clears
                // only its scissored Workspace region.
                openGLContext.clear();
                ui.beginFrame();
                shell.draw(workspace.projectionMode());
                workspace.updateNavigation(shell.workspaceRect(), shell.workspaceInput());
                const auto framebuffer = window.framebufferSize();
                workspace.render(shell.workspaceRect(), framebuffer.width, framebuffer.height);
                ui.endFrame();
                window.swapBuffers();
            }
        }

        microsw::Logger::info("Application window closed");
        microsw::Logger::info("Application shutting down");
        return 0;
    }
    catch (const std::exception& error)
    {
        microsw::Logger::error(error.what());
        microsw::Logger::info("Application shutting down");
        return 1;
    }
}
