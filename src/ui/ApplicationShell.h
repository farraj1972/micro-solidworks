#pragma once

namespace microsw
{
class ApplicationWindow;

class ApplicationShell
{
public:
    explicit ApplicationShell(ApplicationWindow& window);

    void draw();

private:
    void drawMainMenu();
    void drawModelPanel();
    void drawWorkspace();
    void drawStatusBar();
    void drawAboutDialog();

    ApplicationWindow& window_;
    bool aboutDialogRequested_{false};
};
}
