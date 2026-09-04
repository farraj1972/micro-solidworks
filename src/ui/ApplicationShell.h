#pragma once

#include "app/WorkspaceLayout.h"
#include "app/WorkspaceInput.h"

namespace microsw
{
class ApplicationWindow;

class ApplicationShell
{
public:
    explicit ApplicationShell(ApplicationWindow& window);

    void draw(ProjectionMode projectionMode);
    [[nodiscard]] const WorkspaceLayout& workspaceRect() const noexcept { return workspace_; }
    [[nodiscard]] const WorkspaceInput& workspaceInput() const noexcept { return input_; }

private:
    void drawMainMenu(ProjectionMode projectionMode);
    void drawModelPanel();
    void drawWorkspace();
    void drawStatusBar();
    void drawAboutDialog();

    ApplicationWindow& window_;
    WorkspaceLayout workspace_{};
    WorkspaceInput input_{};
    bool aboutDialogRequested_{false};
};
}
