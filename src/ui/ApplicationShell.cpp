#include "ui/ApplicationShell.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"

#include <imgui.h>
#include <numbers>
#include <stdexcept>

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

void ApplicationShell::draw(ProjectionMode projectionMode, const presentation::VisualEntity* selected)
{
    input_ = {};
    transformRequest_.reset();
    drawMainMenu(projectionMode);
    drawModelPanel(selected);
    drawWorkspace();
    drawStatusBar();
    drawAboutDialog();
    const auto& io = ImGui::GetIO();
    const auto* viewport = ImGui::GetMainViewport();
    input_.x = static_cast<double>(io.MousePos.x) - viewport->Pos.x;
    input_.y = static_cast<double>(io.MousePos.y) - viewport->Pos.y;
    input_.leftPressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
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

void ApplicationShell::drawModelPanel(const presentation::VisualEntity* selected)
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
    ImGui::Separator();
    const auto selectedId = selected ? std::optional{selected->id()} : std::nullopt;
    if (selectedId != editorEntity_)
    {
        editorEntity_ = selectedId;
        transformError_.clear();
    }
    if (!selected)
        ImGui::TextWrapped("Select a Point, Segment or Line to edit its transform.");
    else
    {
        ImGui::TextUnformatted("Transform");
        constexpr double degreesPerRadian = 180.0 / std::numbers::pi;
        const auto& transform = selected->transform();
        double translation[]{transform.translation().x(), transform.translation().y(), transform.translation().z()};
        double rotation[]{transform.rotation().x() * degreesPerRadian,
            transform.rotation().y() * degreesPerRadian, transform.rotation().z() * degreesPerRadian};
        double scale[]{transform.scale().x(), transform.scale().y(), transform.scale().z()};
        ImGui::TextDisabled("X                  Y                  Z");
        ImGui::TextUnformatted("Translation");
        ImGui::SetNextItemWidth(-1);
        const bool translationChanged = ImGui::InputScalarN("##Translation", ImGuiDataType_Double, translation, 3,
            nullptr, nullptr, "%.6g");
        ImGui::TextUnformatted("Rotation (degrees)");
        ImGui::SetNextItemWidth(-1);
        const bool rotationChanged = ImGui::InputScalarN("##Rotation", ImGuiDataType_Double, rotation, 3,
            nullptr, nullptr, "%.6g");
        ImGui::TextUnformatted("Scale");
        ImGui::SetNextItemWidth(-1);
        const bool scaleChanged = ImGui::InputScalarN("##Scale", ImGuiDataType_Double, scale, 3,
            nullptr, nullptr, "%.6g");
        ImGui::TextWrapped("Valid edits apply immediately. Scale must be positive.");
        if (translationChanged || rotationChanged || scaleChanged)
        {
            try
            {
                auto candidate = transform;
                if (translationChanged) candidate.setTranslation({translation[0], translation[1], translation[2]});
                if (rotationChanged) candidate.setRotation({rotation[0] / degreesPerRadian,
                    rotation[1] / degreesPerRadian, rotation[2] / degreesPerRadian});
                if (scaleChanged) candidate.setScale({scale[0], scale[1], scale[2]});
                transformRequest_ = candidate;
                transformError_.clear();
            }
            catch (const std::invalid_argument& error) { transformError_ = error.what(); }
        }
        if (!transformError_.empty())
            ImGui::TextWrapped("Edit rejected: %s", transformError_.c_str());
    }
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
