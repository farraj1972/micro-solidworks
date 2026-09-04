#include "ui/ApplicationShell.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"

#include <imgui.h>

namespace
{
constexpr float modelPanelWidth = 260.0F;
constexpr float statusBarHeight = 26.0F;

constexpr ImGuiWindowFlags structuralWindowFlags =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoBringToFrontOnFocus;

void beginStructuralWindow(const char* name, const ImVec2& position, const ImVec2& size)
{
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGui::Begin(name, nullptr, structuralWindowFlags);
}
}

namespace microsw
{
ApplicationShell::ApplicationShell(ApplicationWindow& window)
    : window_{window}
{
    Logger::info("Application shell initialized");
}

void ApplicationShell::draw(ProjectionMode projectionMode)
{
    input_ = {};
    drawMainMenu(projectionMode);
    drawModelPanel();
    drawWorkspace();
    drawStatusBar();
    drawAboutDialog();
    const auto& io = ImGui::GetIO();
    const auto* viewport = ImGui::GetMainViewport();
    input_.x = static_cast<double>(io.MousePos.x) - viewport->Pos.x;
    input_.y = static_cast<double>(io.MousePos.y) - viewport->Pos.y;
    input_.middlePressed = ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
    input_.middleDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    input_.shiftDown = io.KeyShift;
    input_.focused = !io.AppFocusLost;
    input_.pointerValid = ImGui::IsMousePosValid();
    input_.wheelDelta = io.MouseWheel;
    // The Workspace itself legitimately requests mouse capture. Its default
    // hovered test permits starts there, while respecting other windows/items.
    // Popups (including About) and active UI interactions also cancel a drag.
    input_.blocked = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)
        || (io.WantCaptureMouse && ImGui::IsAnyItemActive());
}

void ApplicationShell::drawMainMenu(ProjectionMode projectionMode)
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }

    if (ImGui::BeginMenu("File"))
    {
        ImGui::BeginDisabled();
        ImGui::MenuItem("New");
        ImGui::MenuItem("Open...");
        ImGui::MenuItem("Save");
        ImGui::EndDisabled();
        ImGui::Separator();
        if (ImGui::MenuItem("Exit"))
        {
            Logger::info("Exit requested");
            window_.requestClose();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        ImGui::BeginDisabled();
        ImGui::MenuItem("Undo");
        ImGui::MenuItem("Redo");
        ImGui::EndDisabled();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        if (ImGui::BeginMenu("Projection"))
        {
            if (ImGui::MenuItem("Perspective", nullptr, projectionMode == ProjectionMode::Perspective))
                input_.projectionRequest = ProjectionMode::Perspective;
            if (ImGui::MenuItem("Orthographic", nullptr, projectionMode == ProjectionMode::Orthographic))
                input_.projectionRequest = ProjectionMode::Orthographic;
            ImGui::EndMenu();
        }
        ImGui::BeginDisabled();
        ImGui::MenuItem("Reset Layout");
        ImGui::EndDisabled();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("About"))
        {
            aboutDialogRequested_ = true;
            Logger::info("About opened");
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void ApplicationShell::drawModelPanel()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 contentSize{
        viewport->WorkSize.x,
        viewport->WorkSize.y - statusBarHeight};

    beginStructuralWindow(
        "Model",
        viewport->WorkPos,
        ImVec2{modelPanelWidth, contentSize.y});
    ImGui::TextUnformatted("Model");
    ImGui::Separator();
    ImGui::TextDisabled("No document");
    ImGui::End();
}

void ApplicationShell::drawWorkspace()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 position{viewport->WorkPos.x + modelPanelWidth, viewport->WorkPos.y};
    const ImVec2 size{
        viewport->WorkSize.x - modelPanelWidth,
        viewport->WorkSize.y - statusBarHeight};

    const ImGuiIO& io = ImGui::GetIO();
    workspace_ = {position.x - viewport->Pos.x, position.y - viewport->Pos.y,
                  size.x, size.y, io.DisplaySize.x, io.DisplaySize.y};
    if (size.x <= 0.0F || size.y <= 0.0F)
        return;

    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    // Reserve the UI region without covering the directly rendered 3D content.
    ImGui::Begin("Workspace", nullptr, structuralWindowFlags |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextDisabled("Workspace");
    input_.workspaceHovered = ImGui::IsWindowHovered();
    ImGui::End();
}

void ApplicationShell::drawStatusBar()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 position{
        viewport->WorkPos.x,
        viewport->WorkPos.y + viewport->WorkSize.y - statusBarHeight};

    beginStructuralWindow(
        "Status Bar",
        position,
        ImVec2{viewport->WorkSize.x, statusBarHeight});
    ImGui::TextUnformatted("Ready");
    ImGui::End();
}

void ApplicationShell::drawAboutDialog()
{
    if (aboutDialogRequested_)
    {
        ImGui::OpenPopup("About Micro SolidWorks");
        aboutDialogRequested_ = false;
    }

    if (ImGui::BeginPopupModal(
            "About Micro SolidWorks",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Micro SolidWorks");
        ImGui::Separator();
        ImGui::TextUnformatted("Educational 3D CAD project");
        ImGui::TextUnformatted("Baseline B0 - Foundation");

        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
}
