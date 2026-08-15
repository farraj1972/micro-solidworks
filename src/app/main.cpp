#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "ui/ImGuiLayer.h"

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

            while (!window.shouldClose())
            {
                window.pollEvents();
                ui.beginFrame();
                ui.drawDiagnosticPanel();
                openGLContext.clear();
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
