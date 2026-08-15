#pragma once

namespace microsw
{
class ApplicationWindow;

class OpenGLContext
{
public:
    explicit OpenGLContext(ApplicationWindow& window);

    void clear();

private:
    ApplicationWindow& window_;
};
}
