#include "rendering/OpenGLContext.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"

#include <glad/gl.h>

#include <stdexcept>
#include <string>

namespace microsw
{
namespace
{
GLADapiproc loadOpenGLProcedure(void* userPointer, const char* name)
{
    auto& window = *static_cast<ApplicationWindow*>(userPointer);
    return reinterpret_cast<GLADapiproc>(window.graphicsProcedureAddress(name));
}
}

OpenGLContext::OpenGLContext(ApplicationWindow& window)
    : window_{window}
{
    Logger::info("Making OpenGL context current");
    window_.makeContextCurrent();
    Logger::info("OpenGL context current");

    Logger::info("Loading OpenGL functions");
    if (gladLoadGLUserPtr(loadOpenGLProcedure, &window_) == 0)
    {
        Logger::error("Failed to load OpenGL functions");
        throw std::runtime_error{"GLAD initialization failed"};
    }

    if (GLAD_GL_VERSION_3_3 == 0)
    {
        Logger::error("OpenGL 3.3 is not available");
        throw std::runtime_error{"OpenGL 3.3 is required"};
    }

    const auto* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (version == nullptr)
    {
        Logger::error("Unable to query OpenGL version");
        throw std::runtime_error{"OpenGL version query failed"};
    }

    Logger::info(std::string{"OpenGL version: "} + version);
    Logger::info("OpenGL initialized");
}

void OpenGLContext::clear()
{
    const FramebufferSize size = window_.framebufferSize();
    glViewport(0, 0, size.width, size.height);
    glClearColor(0.12F, 0.12F, 0.14F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
}
}
