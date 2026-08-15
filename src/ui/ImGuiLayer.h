#pragma once

namespace microsw
{
class ApplicationWindow;
class OpenGLContext;

class ImGuiLayer
{
public:
    ImGuiLayer(ApplicationWindow& window, OpenGLContext& openGLContext);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;
    ImGuiLayer(ImGuiLayer&&) = delete;
    ImGuiLayer& operator=(ImGuiLayer&&) = delete;

    void beginFrame();
    void endFrame();

private:
    void shutdown() noexcept;

    bool contextCreated_{false};
    bool glfwBackendInitialized_{false};
    bool openGLBackendInitialized_{false};
};
}
