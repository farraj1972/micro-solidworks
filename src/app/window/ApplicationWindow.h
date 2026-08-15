#pragma once

#include <memory>
#include <string_view>

namespace microsw
{
struct FramebufferSize
{
    int width;
    int height;
};

class ApplicationWindow
{
public:
    using GraphicsProcedure = void (*)();

    ApplicationWindow(int width, int height, std::string_view title);
    ~ApplicationWindow();

    ApplicationWindow(const ApplicationWindow&) = delete;
    ApplicationWindow& operator=(const ApplicationWindow&) = delete;
    ApplicationWindow(ApplicationWindow&&) = delete;
    ApplicationWindow& operator=(ApplicationWindow&&) = delete;

    [[nodiscard]] bool shouldClose() const;
    void requestClose();
    void makeContextCurrent();
    [[nodiscard]] GraphicsProcedure graphicsProcedureAddress(const char* name) const;
    [[nodiscard]] FramebufferSize framebufferSize() const;
    [[nodiscard]] void* nativeHandle() const;
    void swapBuffers();
    void pollEvents();

private:
    class Implementation;
    std::unique_ptr<Implementation> implementation_;
};
}
