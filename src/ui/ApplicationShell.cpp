#include "ui/ApplicationShell.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"

#include <imgui.h>
#include <cmath>
#include <numbers>
#include <string_view>
#include <stdexcept>
#include <type_traits>

namespace
{
constexpr float defaultModelPanelWidth = 320.0F;
constexpr float minimumModelPanelWidth = 260.0F;
constexpr float statusBarHeight = 26.0F;
constexpr float commandBarHeight = 58.0F;

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
    newDocumentRequest_ = openDocumentRequest_ = saveDocumentRequest_ = saveAsDocumentRequest_ = false;
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
    input_.rightPressed = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    input_.escapePressed = ImGui::IsKeyPressed(ImGuiKey_Escape);
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

void ApplicationShell::drawSketch(ProjectionMode projectionMode, SketchTool activeTool,
                                  const sketch::Sketch& sketch, const sketch::SketchEntity* selected,
                                  bool hasActiveSolid, const features::FeatureModel* featureModel,
                                  interaction::EditorContext* editor)
{
    input_ = {};
    sketchToolRequest_.reset();
    sketchGeometryRequest_.reset();
    deleteSketchRequest_ = false;
    addConstraintRequest_.reset(); removeConstraintRequest_.reset();
    drivingValueRequest_.reset(); solveSketchRequest_ = false;
    extrusionRequest_ = false;
    loadBooleanDemoRequest_ = false;
    loadBooleanUnsupportedRequest_ = false;
    assignOperandARequest_ = assignOperandBRequest_ = false;
    booleanRequest_.reset();
    featureDistanceRequest_.reset(); featureBooleanRequest_.reset(); rebuildFeaturesRequest_ = false;
    newDocumentRequest_ = openDocumentRequest_ = saveDocumentRequest_ = saveAsDocumentRequest_ = false;
    editorCommandRequest_.reset(); enterSketchRequest_ = exitSketchRequest_ = false;
    confirmPropertyRequest_ = cancelPropertyRequest_ = false; visibilityToggleRequest_.reset();
    drawMainMenu(projectionMode);
    drawCommandBar(activeTool, sketch, selected, featureModel, editor);
    drawSketchPanel(activeTool, sketch, selected, hasActiveSolid, featureModel, editor);
    drawWorkspace();
    drawStatusBar();
    drawAboutDialog();
    const auto& io = ImGui::GetIO();
    const auto* viewport = ImGui::GetMainViewport();
    input_.x = static_cast<double>(io.MousePos.x) - viewport->Pos.x;
    input_.y = static_cast<double>(io.MousePos.y) - viewport->Pos.y;
    input_.leftPressed = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    input_.rightPressed = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    input_.escapePressed = ImGui::IsKeyPressed(ImGuiKey_Escape);
    input_.middlePressed = ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
    input_.middleDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    input_.shiftDown = io.KeyShift;
    input_.focused = !io.AppFocusLost;
    input_.pointerValid = ImGui::IsMousePosValid();
    input_.wheelDelta = io.MouseWheel;
    input_.blocked = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)
        || (io.WantCaptureMouse && ImGui::IsAnyItemActive());
}

void ApplicationShell::drawCommandBar(SketchTool activeTool, const sketch::Sketch& model,
                                      const sketch::SketchEntity* selected,
                                      const features::FeatureModel* featureModel,
                                      interaction::EditorContext* editor)
{
    const auto* viewport = ImGui::GetMainViewport();
    beginStructuralWindow("CAD Command Bar", viewport->WorkPos,
        ImVec2{viewport->WorkSize.x, commandBarHeight});
    const auto commandButton = [&](const char* icon, const char* tooltip, bool enabled, const auto& action)
    {
        if (!enabled) ImGui::BeginDisabled();
        if (ImGui::Button(icon, ImVec2{34, 30})) action();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s", tooltip);
        if (!enabled) ImGui::EndDisabled();
        ImGui::SameLine();
    };
    ImGui::TextDisabled("General"); ImGui::SameLine();
    commandButton("N", "New document", true, [&]{ newDocumentRequest_ = true; });
    commandButton("O", "Open .microsw", true, [&]{ openDocumentRequest_ = true; });
    commandButton("S", "Save", true, [&]{ saveDocumentRequest_ = true; });
    commandButton("SA", "Save As", true, [&]{ saveAsDocumentRequest_ = true; });
    commandButton("R", "Rebuild model", featureModel != nullptr, [&]{ rebuildFeaturesRequest_ = true; });
    ImGui::Separator(); ImGui::SameLine(); ImGui::TextDisabled("Sketch"); ImGui::SameLine();
    const bool editing = editor && editor->activeSketchFeatureId();
    commandButton(editing ? "X" : "SK", editing ? "Exit Sketch" : "Create or edit Sketch", true,
        [&]{ if (editing) exitSketchRequest_ = true; else enterSketchRequest_ = true; });
    const auto tool = [&](const char* icon, const char* tip, SketchTool sketchTool, interaction::EditorCommand command)
    {
        commandButton(icon, tip, editing, [&]{ sketchToolRequest_ = sketchTool; editorCommandRequest_ = command; });
    };
    tool("SEL", "Select", SketchTool::Select, interaction::EditorCommand::Select);
    tool("L", "Line", SketchTool::Line, interaction::EditorCommand::Line);
    tool("[]", "Rectangle", SketchTool::Rectangle, interaction::EditorCommand::Rectangle);
    tool("O", "Circle", SketchTool::Circle, interaction::EditorCommand::Circle);
    const bool selectedLine = selected && selected->type() == sketch::SketchEntityType::Line;
    commandButton("H", "Horizontal constraint", editing && selectedLine, [&]{
        editorCommandRequest_ = interaction::EditorCommand::Horizontal;
        addConstraintRequest_ = sketch::Horizontal{{selected->id(), sketch::SubElementKind::LineBody}};
    });
    commandButton("V", "Vertical constraint", editing && selectedLine, [&]{
        editorCommandRequest_ = interaction::EditorCommand::Vertical;
        addConstraintRequest_ = sketch::Vertical{{selected->id(), sketch::SubElementKind::LineBody}};
    });
    commandButton("C", "Coincident: select two endpoints", editing, [&]{ editorCommandRequest_ = interaction::EditorCommand::Coincident; });
    commandButton("D", "Driving dimension", editing && selected, [&]{ editorCommandRequest_ = interaction::EditorCommand::Dimension; });
    ImGui::Separator(); ImGui::SameLine(); ImGui::TextDisabled("Features"); ImGui::SameLine();
    const bool canExtrude = editor && editor->commandAvailable(interaction::EditorCommand::Extrude, model.size() >= 3);
    commandButton("EX", "Extrude selected Sketch", canExtrude, [&]{ editorCommandRequest_ = interaction::EditorCommand::Extrude; });
    ImGui::Separator(); ImGui::SameLine(); ImGui::TextDisabled("View: mouse navigation");
    ImGui::End();
    (void)activeTool;
}

void ApplicationShell::drawSketchPanel(SketchTool activeTool, const sketch::Sketch& model,
                                       const sketch::SketchEntity* selected, bool hasActiveSolid,
                                       const features::FeatureModel* featureModel,
                                       interaction::EditorContext* editor)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 contentSize{viewport->WorkSize.x, viewport->WorkSize.y - statusBarHeight - commandBarHeight};
    ImGui::SetNextWindowPos(ImVec2{viewport->WorkPos.x, viewport->WorkPos.y + commandBarHeight});
    ImGui::SetNextWindowSize(ImVec2{defaultModelPanelWidth, contentSize.y}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(
        ImVec2{minimumModelPanelWidth, contentSize.y},
        ImVec2{viewport->WorkSize.x * 0.7F, contentSize.y});
    constexpr ImGuiWindowFlags sketchPanelFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Model", nullptr, sketchPanelFlags);
    modelPanelWidth_ = ImGui::GetWindowSize().x;
    ImGui::TextUnformatted("Document");
    ImGui::InputText("Path (.microsw)", documentPathInput_.data(), documentPathInput_.size());
    ImGui::TextWrapped("Current: %s", currentFilePath_.empty() ? "Untitled" : currentFilePath_.c_str());
    if (!persistenceStatus_.empty()) ImGui::TextWrapped("%s", persistenceStatus_.c_str());
    ImGui::Separator();
    if (featureModel)
    {
        ImGui::TextUnformatted("Feature Manager");
        ImGui::TextDisabled("Part / Model");
        const auto stateName = [](features::FeatureState state) {
            switch (state) { case features::FeatureState::Clean: return "OK"; case features::FeatureState::Dirty: return "Needs rebuild";
            case features::FeatureState::Failed: return "Failed"; default: return "Blocked"; }
        };
        for (const auto& feature : featureModel->history())
        {
            ImGui::PushID(static_cast<int>(feature.id().value()));
            const auto display = editor ? editor->displayState(feature.id()) : interaction::DisplayState::Visible;
            const std::string label = std::string{display == interaction::DisplayState::Hidden ? "[ ] " : "[o] "}
                + feature.displayName() + "  -  " + stateName(feature.state());
            if (ImGui::Selectable(label.c_str(), selectedFeature_ == feature.id()))
            {
                selectedFeature_ = feature.id();
                if (editor) editor->select(interaction::LogicalTarget{feature.id()});
            }
            if (ImGui::BeginPopupContextItem("Feature actions"))
            {
                if (feature.kind() == features::FeatureKind::Sketch && ImGui::MenuItem("Edit Sketch"))
                { selectedFeature_ = feature.id(); enterSketchRequest_ = true; }
                if (feature.kind() == features::FeatureKind::Sketch && ImGui::MenuItem("Extrude"))
                { selectedFeature_ = feature.id(); editorCommandRequest_ = interaction::EditorCommand::Extrude; }
                if (ImGui::MenuItem(display == interaction::DisplayState::Hidden ? "Show" : "Hide"))
                    visibilityToggleRequest_ = feature.id();
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        if (selectedFeature_)
        {
            const auto& feature = featureModel->find(*selectedFeature_);
            ImGui::Separator(); ImGui::Text("Selected: %s", feature.displayName().c_str());
            if (feature.kind() == features::FeatureKind::Sketch)
            {
                const auto& sketch = std::get<features::SketchFeature>(feature.data()).sketch;
                ImGui::Text("%zu entities, %zu constraints", sketch.size(), sketch.constraintCount());
                if (ImGui::Button("Edit Sketch")) enterSketchRequest_ = true;
                ImGui::SameLine();
                if (ImGui::Button("Extrude")) editorCommandRequest_ = interaction::EditorCommand::Extrude;
            }
            else if (feature.kind() == features::FeatureKind::Extrude)
            {
                const auto& extrude = std::get<features::ExtrudeFeature>(feature.data());
                double distance = extrude.distance;
                ImGui::TextUnformatted("Property Manager — Extrude");
                ImGui::InputDouble("Distance", &distance);
                if (ImGui::Button("Confirm")) { featureDistanceRequest_ = std::pair{feature.id(), distance}; rebuildFeaturesRequest_ = true; }
                ImGui::SameLine(); if (ImGui::Button("Cancel")) cancelPropertyRequest_ = true;
            }
            else
            {
                ImGui::TextUnformatted("Boolean result");
            }
            if (!feature.diagnostic().empty()) ImGui::TextWrapped("%s", feature.diagnostic().c_str());
        }
        if (!featureRebuildStatus_.empty()) ImGui::TextWrapped("%s", featureRebuildStatus_.c_str());
        ImGui::Separator();
    }
    ImGui::TextUnformatted(editor && editor->activeSketchFeatureId() ? "Property / Tool Manager — Sketch" : "Property / Tool Manager");
    const char* toolName = activeTool == SketchTool::Select ? "Select" : activeTool == SketchTool::Line ? "Line"
        : activeTool == SketchTool::Rectangle ? "Rectangle" : activeTool == SketchTool::Circle ? "Circle" : "Arc";
    ImGui::Text("Current tool: %s", toolName);
    if (editor && editor->activeSketchFeatureId())
    {
        ImGui::Checkbox("Grid snap", &gridSnapEnabled_);
        ImGui::TextWrapped("Click in the Graphics Area. Esc cancels the current step, then returns to Select.");
        if (editor->snapCandidate())
        {
            const char* snap = editor->snapCandidate()->kind == interaction::SnapKind::Endpoint ? "Endpoint snap"
                : editor->snapCandidate()->kind == interaction::SnapKind::Midpoint ? "Midpoint snap" : "Grid snap";
            ImGui::TextColored(ImVec4{0.25F, 0.9F, 1.0F, 1.0F}, "%s", snap);
        }
        if (editor->inferenceCandidate())
        {
            const char* cue = editor->inferenceCandidate()->kind == interaction::InferenceKind::Horizontal ? "Horizontal inference"
                : editor->inferenceCandidate()->kind == interaction::InferenceKind::Vertical ? "Vertical inference" : "Coincident inference";
            ImGui::TextColored(ImVec4{0.4F, 1.0F, 0.45F, 1.0F}, "%s", cue);
        }
    }
    ImGui::Separator();

    const auto selectedId = selected ? std::optional{selected->id()} : std::nullopt;
    if (selectedId != sketchEditorEntity_) { sketchEditorEntity_ = selectedId; sketchError_.clear(); }
    if (!selected) ImGui::TextWrapped("Choose a tool or select a sketch entity.");
    else
    {
        std::visit([&](const auto& typed)
        {
            using Entity = std::decay_t<decltype(typed)>;
            if constexpr (std::is_same_v<Entity, sketch::SketchLine>)
            {
                double values[]{typed.geometry.a().x(), typed.geometry.a().y(),
                                typed.geometry.b().x(), typed.geometry.b().y()};
                ImGui::TextUnformatted("Line: start X/Y, end X/Y");
                if (ImGui::InputScalarN("##SketchLine", ImGuiDataType_Double, values, 4))
                    try { sketchGeometryRequest_ = sketch::SketchLine{{{values[0], values[1]}, {values[2], values[3]}}}; }
                    catch (const std::exception& e) { sketchError_ = e.what(); }
            }
            else if constexpr (std::is_same_v<Entity, sketch::SketchCircle>)
            {
                double values[]{typed.geometry.center().x(), typed.geometry.center().y(), typed.geometry.radius()};
                ImGui::TextUnformatted("Circle: center X/Y, radius");
                if (ImGui::InputScalarN("##SketchCircle", ImGuiDataType_Double, values, 3))
                    try { sketchGeometryRequest_ = sketch::SketchCircle{{{values[0], values[1]}, values[2]}}; }
                    catch (const std::exception& e) { sketchError_ = e.what(); }
            }
            else
            {
                constexpr double degreesPerRadian = 180.0 / std::numbers::pi;
                double values[]{typed.geometry.center().x(), typed.geometry.center().y(), typed.geometry.radius(),
                    typed.geometry.startAngle() * degreesPerRadian, typed.geometry.sweepAngle() * degreesPerRadian};
                ImGui::TextUnformatted("Arc: center X/Y, radius, start/sweep degrees");
                if (ImGui::InputScalarN("##SketchArc", ImGuiDataType_Double, values, 5))
                    try { sketchGeometryRequest_ = sketch::SketchArc{{{values[0], values[1]}, values[2],
                        values[3] / degreesPerRadian, values[4] / degreesPerRadian}}; }
                    catch (const std::exception& e) { sketchError_ = e.what(); }
            }
        }, selected->geometry());
        if (ImGui::Button("Delete") || ImGui::IsKeyPressed(ImGuiKey_Delete)) deleteSketchRequest_ = true;
    }
    if (!sketchError_.empty()) ImGui::TextWrapped("Edit rejected: %s", sketchError_.c_str());
    ImGui::Separator();
    ImGui::Text("Sketch contents: %zu entities, %zu constraints", model.size(), model.constraintCount());
    if (editor && editor->activeCommand() == interaction::EditorCommand::Coincident)
    {
        ImGui::Text("Coincident: %zu/2 points selected", editor->operands().size());
        if (editor->operands().size() == 2 && ImGui::Button("Confirm Coincident"))
        {
            addConstraintRequest_ = sketch::Coincident{editor->operands()[0], editor->operands()[1]};
            confirmPropertyRequest_ = true;
        }
        ImGui::SameLine(); if (ImGui::Button("Cancel##Coincident")) cancelPropertyRequest_ = true;
    }
    if (selected)
    {
        const auto id = selected->id();
        const auto dimensionFor = [&](sketch::SketchEntityId entityId, bool radius) {
            std::optional<sketch::SketchConstraintId> result;
            for (const auto constraintId : model.constraintIds())
            {
                std::visit([&](const auto& constraint) {
                    using T = std::decay_t<decltype(constraint)>;
                    if constexpr (std::is_same_v<T, sketch::LineLength>)
                    {
                        if (!radius && constraint.line.entity == entityId) result = constraintId;
                    }
                    else if constexpr (std::is_same_v<T, sketch::CircleRadius>)
                    {
                        if (radius && constraint.radius.entity == entityId) result = constraintId;
                    }
                }, model.findConstraint(constraintId).value());
            }
            return result;
        };
        if (selected->type() == sketch::SketchEntityType::Line)
        {
            const sketch::SketchElementRef ref{id, sketch::SubElementKind::LineBody};
            if (ImGui::Button("Add Horizontal")) addConstraintRequest_ = sketch::Horizontal{ref};
            if (ImGui::Button("Add Vertical")) addConstraintRequest_ = sketch::Vertical{ref};
            const auto& line = std::get<sketch::SketchLine>(selected->geometry()).geometry;
            if (const auto constraintId = dimensionFor(id, false))
            {
                double length = std::get<sketch::LineLength>(model.findConstraint(*constraintId).value()).value;
                if (ImGui::InputDouble("Line length constraint", &length))
                    drivingValueRequest_ = std::pair{*constraintId, length};
            }
            else
            {
                ImGui::Text("Current line length: %.6g", line.length());
                if (ImGui::Button("Add line length constraint"))
                    addConstraintRequest_ = sketch::LineLength{ref, line.length()};
            }
        }
        else if (selected->type() == sketch::SketchEntityType::Circle)
        {
            const auto radiusRef = sketch::SketchElementRef{id, sketch::SubElementKind::CircleRadius};
            const auto& circle = std::get<sketch::SketchCircle>(selected->geometry()).geometry;
            if (const auto constraintId = dimensionFor(id, true))
            {
                double radius = std::get<sketch::CircleRadius>(model.findConstraint(*constraintId).value()).value;
                if (ImGui::InputDouble("Radius constraint", &radius))
                    drivingValueRequest_ = std::pair{*constraintId, radius};
            }
            else
            {
                ImGui::Text("Current radius: %.6g", circle.radius());
                if (ImGui::Button("Add radius constraint"))
                    addConstraintRequest_ = sketch::CircleRadius{radiusRef, circle.radius()};
            }
        }
    }
    ImGui::Separator(); ImGui::TextUnformatted("Constraint glyphs / dimensions");
    for (const auto id : model.constraintIds())
    {
        ImGui::PushID(static_cast<int>(id.value()));
        const auto& value = model.findConstraint(id).value();
        const char* name = std::visit([](const auto& c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, sketch::Coincident>) return "Coincident";
            else if constexpr (std::is_same_v<T, sketch::Horizontal>) return "Horizontal";
            else if constexpr (std::is_same_v<T, sketch::Vertical>) return "Vertical";
            else if constexpr (std::is_same_v<T, sketch::Parallel>) return "Parallel";
            else if constexpr (std::is_same_v<T, sketch::Perpendicular>) return "Perpendicular";
            else if constexpr (std::is_same_v<T, sketch::HorizontalDistance>) return "HorizontalDistance";
            else if constexpr (std::is_same_v<T, sketch::VerticalDistance>) return "VerticalDistance";
            else if constexpr (std::is_same_v<T, sketch::LineLength>) return "LineLength";
            else return "CircleRadius";
        }, value);
        const char* glyph = std::string_view{name} == "Horizontal" ? "[H]" : std::string_view{name} == "Vertical" ? "[V]"
            : std::string_view{name} == "Coincident" ? "[o]" : "[D]";
        ImGui::Text("%s %s", glyph, name);
        if (sketch::isDrivingDimension(value))
        {
            double driving = std::visit([](const auto& c) -> double {
                using T = std::decay_t<decltype(c)>;
                if constexpr (std::is_same_v<T, sketch::HorizontalDistance>
                    || std::is_same_v<T, sketch::VerticalDistance>
                    || std::is_same_v<T, sketch::LineLength>
                    || std::is_same_v<T, sketch::CircleRadius>) return c.value;
                else return 0;
            }, value);
            ImGui::InputDouble("Value", &driving);
            if (ImGui::Button("Apply dimension")) drivingValueRequest_ = std::pair{id, driving};
        }
        if (ImGui::Button("Remove constraint")) removeConstraintRequest_ = id;
        ImGui::PopID();
    }
    if (ImGui::Button("Solve / Rebuild Sketch")) solveSketchRequest_ = true;
    ImGui::Separator();
    if (editor && editor->activeCommand() == interaction::EditorCommand::Extrude)
    {
        ImGui::TextUnformatted("Property Manager — Extrude");
        ImGui::TextUnformatted("Source: selected Sketch");
        ImGui::InputDouble("Distance", &extrusionDistance_);
        const bool valid = std::isfinite(extrusionDistance_) && extrusionDistance_ > 0 && model.size() >= 3;
        if (!valid) ImGui::TextColored(ImVec4{1,0.35F,0.25F,1}, "A closed Sketch and positive distance are required.");
        if (!valid) ImGui::BeginDisabled();
        if (ImGui::Button("Confirm Extrude")) { extrusionRequest_ = true; confirmPropertyRequest_ = true; }
        if (!valid) ImGui::EndDisabled();
        ImGui::SameLine(); if (ImGui::Button("Cancel##Extrude")) cancelPropertyRequest_ = true;
    }
    if (hasActiveSolid) ImGui::TextUnformatted("Modeled result available");
    if (!extrusionStatus_.empty()) ImGui::TextWrapped("%s", extrusionStatus_.c_str());
    ImGui::End();
}

void ApplicationShell::reportFeatureRebuild(const features::RebuildResult& result)
{
    if (result.status == features::RebuildStatus::Success) featureRebuildStatus_ = "Rebuild PASS";
    else if (result.status == features::RebuildStatus::SuccessEmpty) featureRebuildStatus_ = "Rebuild PASS: final result EMPTY";
    else featureRebuildStatus_ = "Rebuild FAILED at Feature "
        + (result.failedFeature ? std::to_string(result.failedFeature->value()) : std::string{"?"})
        + "; blocked downstream: " + std::to_string(result.blockedCount) + "; " + result.diagnostic;
}

void ApplicationShell::reportBooleanStatus(modeling::BooleanStatus status)
{
    switch (status)
    {
    case modeling::BooleanStatus::SUCCESS: booleanStatus_ = "SUCCESS"; break;
    case modeling::BooleanStatus::EMPTY: booleanStatus_ = "EMPTY"; break;
    case modeling::BooleanStatus::INVALID_INPUT: booleanStatus_ = "INVALID_INPUT"; break;
    case modeling::BooleanStatus::UNSUPPORTED_CASE: booleanStatus_ = "UNSUPPORTED_CASE"; break;
    case modeling::BooleanStatus::NUMERICAL_FAILURE: booleanStatus_ = "NUMERICAL_FAILURE"; break;
    }
}

void ApplicationShell::drawMainMenu(ProjectionMode projectionMode)
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New")) newDocumentRequest_ = true;
        if (ImGui::MenuItem("Open...")) openDocumentRequest_ = true;
        if (ImGui::MenuItem("Save")) saveDocumentRequest_ = true;
        if (ImGui::MenuItem("Save As...")) saveAsDocumentRequest_ = true;
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
        ImVec2{modelPanelWidth_, contentSize.y});
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
    const ImVec2 position{viewport->WorkPos.x + modelPanelWidth_, viewport->WorkPos.y + commandBarHeight};
    const ImVec2 size{
        viewport->WorkSize.x - modelPanelWidth_,
        viewport->WorkSize.y - statusBarHeight - commandBarHeight};

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
    ImGui::SameLine();
    ImGui::TextColored(ImVec4{1.0F, 0.25F, 0.25F, 1.0F}, "X");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4{0.25F, 1.0F, 0.25F, 1.0F}, "Y");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4{0.3F, 0.55F, 1.0F, 1.0F}, "Z");
    input_.workspaceHovered = ImGui::IsWindowHovered();
    if (input_.workspaceHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::OpenPopup("Graphics context");
    if (ImGui::BeginPopup("Graphics context"))
    {
        if (selectedFeature_)
        {
            if (ImGui::MenuItem("Edit selected Sketch")) enterSketchRequest_ = true;
            if (ImGui::MenuItem("Show / Hide")) visibilityToggleRequest_ = selectedFeature_;
            if (ImGui::MenuItem("Extrude")) editorCommandRequest_ = interaction::EditorCommand::Extrude;
        }
        else ImGui::TextDisabled("No logical object selected");
        if (ImGui::MenuItem("Rebuild")) rebuildFeaturesRequest_ = true;
        ImGui::EndPopup();
    }
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
    ImGui::Text("%s | %s", currentFilePath_.empty() ? "Untitled" : currentFilePath_.c_str(),
        persistenceStatus_.empty() ? "Ready" : persistenceStatus_.c_str());
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
