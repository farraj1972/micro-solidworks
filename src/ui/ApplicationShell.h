#pragma once

#include "app/WorkspaceLayout.h"

namespace microsw
{
class ApplicationWindow;

class ApplicationShell
{
public:
    explicit ApplicationShell(ApplicationWindow& window);

    void draw();
    [[nodiscard]] const WorkspaceLayout& workspaceRect() const noexcept { return workspace_; }

private:
    void drawMainMenu();
    void drawModelPanel();
    void drawWorkspace();
    void drawStatusBar();
    void drawAboutDialog();

    ApplicationWindow& window_;
    WorkspaceLayout workspace_{};
    bool aboutDialogRequested_{false};
};
}
