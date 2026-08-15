#include "app/window/ApplicationWindow.h"

#include "app/logging/Logger.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

namespace microsw
{
class ApplicationWindow::Implementation
{
public:
    Implementation(const int width, const int height, const std::string_view title)
    {
        Logger::info("Initializing GLFW");
        if (glfwInit() != GLFW_TRUE)
        {
            Logger::error("Failed to initialize GLFW");
            throw std::runtime_error{"GLFW initialization failed"};
        }
        glfwInitialized_ = true;

        try
        {
            Logger::info("GLFW initialized");
            Logger::info("Creating OpenGL 3.3 core context and application window");

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

            const std::string windowTitle{title};
            window_ = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
            if (window_ == nullptr)
            {
                Logger::error("Failed to create application window");
                throw std::runtime_error{"Application window creation failed"};
            }

            Logger::info("Application window created");
        }
        catch (...)
        {
            if (window_ != nullptr)
            {
                glfwDestroyWindow(window_);
                window_ = nullptr;
            }

            glfwTerminate();
            glfwInitialized_ = false;
            throw;
        }
    }

    ~Implementation()
    {
        if (window_ != nullptr)
        {
            glfwDestroyWindow(window_);
        }

        if (glfwInitialized_)
        {
            glfwTerminate();
        }
    }

    [[nodiscard]] bool shouldClose() const
    {
        return glfwWindowShouldClose(window_) == GLFW_TRUE;
    }

    void makeContextCurrent()
    {
        glfwMakeContextCurrent(window_);
        if (glfwGetCurrentContext() != window_)
        {
            Logger::error("Failed to make OpenGL context current");
            throw std::runtime_error{"OpenGL context could not be made current"};
        }
    }

    [[nodiscard]] GraphicsProcedure graphicsProcedureAddress(const char* name) const
    {
        return glfwGetProcAddress(name);
    }

    [[nodiscard]] FramebufferSize framebufferSize() const
    {
        FramebufferSize size{};
        glfwGetFramebufferSize(window_, &size.width, &size.height);
        return size;
    }

    void swapBuffers()
    {
        glfwSwapBuffers(window_);
    }

    void pollEvents()
    {
        glfwPollEvents();
    }

private:
    bool glfwInitialized_{false};
    GLFWwindow* window_{nullptr};
};

ApplicationWindow::ApplicationWindow(
    const int width,
    const int height,
    const std::string_view title)
    : implementation_{std::make_unique<Implementation>(width, height, title)}
{
}

ApplicationWindow::~ApplicationWindow() = default;

bool ApplicationWindow::shouldClose() const
{
    return implementation_->shouldClose();
}

void ApplicationWindow::makeContextCurrent()
{
    implementation_->makeContextCurrent();
}

ApplicationWindow::GraphicsProcedure ApplicationWindow::graphicsProcedureAddress(
    const char* name) const
{
    return implementation_->graphicsProcedureAddress(name);
}

FramebufferSize ApplicationWindow::framebufferSize() const
{
    return implementation_->framebufferSize();
}

void ApplicationWindow::swapBuffers()
{
    implementation_->swapBuffers();
}

void ApplicationWindow::pollEvents()
{
    implementation_->pollEvents();
}
}
