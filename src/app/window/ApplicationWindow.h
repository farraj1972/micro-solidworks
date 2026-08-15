#pragma once

#include <memory>
#include <string_view>

namespace microsw
{
class ApplicationWindow
{
public:
    ApplicationWindow(int width, int height, std::string_view title);
    ~ApplicationWindow();

    ApplicationWindow(const ApplicationWindow&) = delete;
    ApplicationWindow& operator=(const ApplicationWindow&) = delete;
    ApplicationWindow(ApplicationWindow&&) = delete;
    ApplicationWindow& operator=(ApplicationWindow&&) = delete;

    [[nodiscard]] bool shouldClose() const;
    void pollEvents();

private:
    class Implementation;
    std::unique_ptr<Implementation> implementation_;
};
}
