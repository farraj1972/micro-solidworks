#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "app/demo/GeometryDemoScene.h"
#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "viewer/WorkspaceViewport.h"

#include <exception>
#include <stdexcept>

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
            auto presentation = microsw::demo::createGeometryDemoScene();
            microsw::viewer::WorkspaceViewport workspace{presentation};

            while (!window.shouldClose())
            {
                window.pollEvents();
                // Whole-frame background before UI composition; the 3D pass clears
                // only its scissored Workspace region.
                openGLContext.clear();
                ui.beginFrame();
                const auto selected = workspace.selectedEntity();
                shell.draw(workspace.projectionMode(), selected ? presentation.find(*selected) : nullptr);
                if (selected && shell.transformRequest())
                {
                    try
                    {
                        workspace.validateTransform(*selected, *shell.transformRequest());
                        presentation.setTransform(*selected, *shell.transformRequest());
                    }
                    catch (const std::invalid_argument& error) { shell.reportTransformError(error.what()); }
                    catch (const std::overflow_error& error) { shell.reportTransformError(error.what()); }
                }
                workspace.updateNavigation(shell.workspaceRect(), shell.workspaceInput());
                const auto framebuffer = window.framebufferSize();
                workspace.updateHover(shell.workspaceRect(), shell.workspaceInput(),
                    framebuffer.width, framebuffer.height);
                workspace.updateSelection(shell.workspaceRect(), shell.workspaceInput(),
                    framebuffer.width, framebuffer.height);
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
