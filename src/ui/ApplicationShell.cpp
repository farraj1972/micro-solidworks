#include "ui/ApplicationShell.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "features/PlanarRegion.h"
#include "ui/FeatureTreeModel.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <numbers>
#include <string_view>
#include <stdexcept>
#include <type_traits>

namespace
{
microsw::features::PlanarRegionSet extrusionRegions(const microsw::sketch::Sketch& sketch)
{
    return microsw::features::extractPlanarRegions(sketch);
}

constexpr float defaultModelPanelWidth = 320.0F;
constexpr float minimumModelPanelWidth = 260.0F;
constexpr float statusBarHeight = 26.0F;
constexpr float commandBarHeight = 88.0F;

enum class CadIcon { New, Open, Save, SaveAs, Rebuild, Sketch, ExitSketch, Select,
    Line, Rectangle, Circle, Horizontal, Vertical, Coincident, Dimension, Extrude, CutExtrude, Undo, Redo };

void drawCadIcon(ImDrawList* draw, CadIcon icon, ImVec2 center, ImU32 color)
{
    const float x = center.x, y = center.y;
    const auto line = [&](float ax, float ay, float bx, float by, float width = 2.0F) {
        draw->AddLine({x + ax, y + ay}, {x + bx, y + by}, color, width);
    };
    const auto circle = [&](float ox, float oy, float radius, float width = 2.0F) {
        draw->AddCircle({x + ox, y + oy}, radius, color, 20, width);
    };
    switch (icon)
    {
    case CadIcon::New:
        draw->AddRect({x-9,y-11},{x+6,y+10},color,1,0,2); line(2,-11,6,-7); line(10,1,10,11); line(5,6,15,6); break;
    case CadIcon::Open:
        line(-12,-5,-3,-5); line(-3,-5,0,-9); line(0,-9,9,-9); line(9,-9,11,-5);
        draw->AddQuad({x-12,y-5},{x+12,y-5},{x+8,y+10},{x-9,y+10},color,2); break;
    case CadIcon::Save:
    case CadIcon::SaveAs:
        draw->AddRect({x-11,y-11},{x+11,y+11},color,1,0,2); draw->AddRect({x-6,y-10},{x+5,y-3},color,0,0,2);
        draw->AddRect({x-6,y+3},{x+6,y+10},color,0,0,2); if(icon==CadIcon::SaveAs){line(8,2,8,12);line(3,7,13,7);} break;
    case CadIcon::Rebuild:
        draw->PathArcTo({x,y},10,-2.6F,1.1F,20); draw->PathStroke(color,0,2.5F);
        draw->AddTriangleFilled({x+10,y-9},{x+12,y-1},{x+4,y-3},color); break;
    case CadIcon::Sketch:
        draw->AddRect({x-11,y-9},{x+7,y+9},color,0,0,2); line(-5,5,11,-11,3); circle(10,-10,2); break;
    case CadIcon::ExitSketch:
        line(-11,0,-3,8,3); line(-3,8,12,-9,3); break;
    case CadIcon::Select:
        draw->AddTriangleFilled({x-10,y-12},{x-7,y+10},{x+1,y+3},color); line(0,2,8,11,3); break;
    case CadIcon::Line:
        line(-11,9,11,-9,3); circle(-11,9,2); circle(11,-9,2); break;
    case CadIcon::Rectangle:
        draw->AddRect({x-11,y-8},{x+11,y+8},color,0,0,2.5F); break;
    case CadIcon::Circle:
        circle(0,0,10,2.5F); circle(0,0,2); break;
    case CadIcon::Horizontal:
        line(-11,0,11,0,3); circle(-11,0,2); circle(11,0,2); line(-7,-7,7,-7); break;
    case CadIcon::Vertical:
        line(0,-11,0,11,3); circle(0,-11,2); circle(0,11,2); line(7,-7,7,7); break;
    case CadIcon::Coincident:
        circle(-4,0,6,2.5F); circle(4,0,6,2.5F); draw->AddCircleFilled({x,y},2,color); break;
    case CadIcon::Dimension:
        line(-10,-7,-10,8); line(10,-7,10,8); line(-10,4,10,4);
        draw->AddTriangleFilled({x-10,y+4},{x-5,y+1},{x-5,y+7},color);
        draw->AddTriangleFilled({x+10,y+4},{x+5,y+1},{x+5,y+7},color); break;
    case CadIcon::Extrude:
        draw->AddRect({x-11,y-5},{x+3,y+9},color,0,0,2); draw->AddRect({x-3,y-11},{x+11,y+3},color,0,0,2);
        line(-11,-5,-3,-11); line(3,-5,11,-11); line(3,9,11,3); break;
    case CadIcon::CutExtrude:
        draw->AddRect({x-11,y-5},{x+3,y+9},color,0,0,2); draw->AddRect({x-3,y-11},{x+11,y+3},color,0,0,2);
        line(-11,-5,-3,-11); line(3,-5,11,-11); line(3,9,11,3);
        circle(0,-1,4,2); line(-3,-4,3,2,2); break;
    case CadIcon::Undo:
    case CadIcon::Redo:
        draw->PathArcTo({x,y+2},9,icon==CadIcon::Undo?-0.5F:-2.6F,icon==CadIcon::Undo?-3.8F:0.6F,18);
        draw->PathStroke(color,0,2.5F);
        if(icon==CadIcon::Undo)draw->AddTriangleFilled({x-10,y-7},{x-11,y+1},{x-3,y-1},color);
        else draw->AddTriangleFilled({x+10,y-7},{x+11,y+1},{x+3,y-1},color);
        break;
    }
}

bool displayModeButton(microsw::DisplayModeRequest mode)
{
    const ImVec2 size{38, 24};
    const ImVec2 min=ImGui::GetCursorScreenPos();
    const ImVec2 max{min.x+size.x,min.y+size.y};
    const bool pressed=ImGui::InvisibleButton("##display-mode",size);
    auto* draw=ImGui::GetWindowDrawList();
    const auto border=ImGui::GetColorU32(ImGui::IsItemHovered()?ImGuiCol_ButtonHovered:ImGuiCol_Button);
    draw->AddRectFilled(min,max,border,4);
    const ImVec2 a{min.x+9,min.y+6},b{max.x-9,max.y-6};
    if(mode!=microsw::DisplayModeRequest::Wireframe)
        draw->AddRectFilled(a,b,IM_COL32(95,145,195,255),2);
    if(mode!=microsw::DisplayModeRequest::Shaded)
        draw->AddRect(a,b,IM_COL32(25,30,38,255),2,0,2);
    if(mode==microsw::DisplayModeRequest::Wireframe)
        draw->AddLine(a,b,IM_COL32(25,30,38,255),2);
    if(ImGui::IsItemHovered()){
        const char* active=mode==microsw::DisplayModeRequest::Shaded?"Shaded"
            :mode==microsw::DisplayModeRequest::Wireframe?"Wireframe":"Shaded With Edges";
        ImGui::SetTooltip("Display mode: %s",active);
    }
    return pressed;
}

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
    viewCommandRequest_.reset(); displayModeRequest_.reset();
    transformRequest_.reset();
    newDocumentRequest_ = openDocumentRequest_ = saveDocumentRequest_ = saveAsDocumentRequest_ = false;
    undoRequest_ = redoRequest_ = exitDocumentRequest_ = false; unsavedDecision_.reset();
    drawMainMenu(projectionMode);
    drawModelPanel(selected);
    drawWorkspace();
    drawStatusBar();
    drawAboutDialog();
    drawUnsavedChangesDialog();
    const auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z) && canUndo_) undoRequest_ = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y) && canRedo_) redoRequest_ = true;
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
    viewCommandRequest_.reset(); displayModeRequest_.reset();
    sketchToolRequest_.reset();
    sketchGeometryRequest_.reset();
    deleteSketchRequest_ = false;
    addConstraintRequest_.reset(); removeConstraintRequest_.reset();
    drivingValueRequest_.reset(); solveSketchRequest_ = false;
    extrusionRequest_ = false;
    cutExtrudeRequest_.reset(); featureCutRequest_.reset();
    loadBooleanDemoRequest_ = false;
    loadBooleanUnsupportedRequest_ = false;
    assignOperandARequest_ = assignOperandBRequest_ = false;
    booleanRequest_.reset();
    featureDistanceRequest_.reset(); featureBooleanRequest_.reset(); rebuildFeaturesRequest_ = false;
    newDocumentRequest_ = openDocumentRequest_ = saveDocumentRequest_ = saveAsDocumentRequest_ = false;
    undoRequest_ = redoRequest_ = exitDocumentRequest_ = false; unsavedDecision_.reset();
    editorCommandRequest_.reset(); enterSketchRequest_ = exitSketchRequest_ = false;
    confirmPropertyRequest_ = cancelPropertyRequest_ = false; visibilityToggleRequest_.reset();
    drawMainMenu(projectionMode);
    drawCommandBar(activeTool, sketch, selected, featureModel, editor);
    drawSketchPanel(activeTool, sketch, selected, hasActiveSolid, featureModel, editor);
    drawWorkspace(activeTool, editor);
    drawStatusBar();
    drawAboutDialog();
    drawUnsavedChangesDialog();
    const auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z) && canUndo_) undoRequest_ = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y) && canRedo_) redoRequest_ = true;
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
    const auto commandButton = [&](const char* id, const char* label, const char* tooltip,
                                   CadIcon icon, bool enabled, bool active, const auto& action)
    {
        ImGui::PushID(id);
        if (!enabled) ImGui::BeginDisabled();
        if (active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.18F, 0.42F, 0.66F, 1.0F});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.22F, 0.50F, 0.76F, 1.0F});
        }
        const bool clicked = ImGui::Button("##tool", ImVec2{62, 58});
        const auto topLeft = ImGui::GetItemRectMin();
        auto* draw = ImGui::GetWindowDrawList();
        const ImU32 color = ImGui::GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        drawCadIcon(draw, icon, {topLeft.x + 31, topLeft.y + 21}, color);
        const auto labelSize = ImGui::CalcTextSize(label);
        draw->AddText({topLeft.x + (62-labelSize.x)/2, topLeft.y + 40}, color, label);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s", tooltip);
        if (active) ImGui::PopStyleColor(2);
        if (!enabled) ImGui::EndDisabled();
        if (clicked && enabled) action();
        ImGui::PopID();
        ImGui::SameLine();
    };
    commandButton("new", "New", "Create a new part", CadIcon::New, true, false, [&]{ newDocumentRequest_ = true; });
    commandButton("open", "Open", "Open a .microsw part", CadIcon::Open, true, false, [&]{ openDocumentRequest_ = true; });
    commandButton("save", "Save", "Save the current part", CadIcon::Save, true, false, [&]{ saveDocumentRequest_ = true; });
    commandButton("saveas", "Save As", "Save the part to a new path", CadIcon::SaveAs, true, false, [&]{ saveAsDocumentRequest_ = true; });
    commandButton("undo", "Undo", "Undo the last committed CAD action (Ctrl+Z)", CadIcon::Undo, canUndo_, false, [&]{ undoRequest_ = true; });
    commandButton("redo", "Redo", "Redo the next committed CAD action (Ctrl+Y)", CadIcon::Redo, canRedo_, false, [&]{ redoRequest_ = true; });
    commandButton("rebuild", "Rebuild", "Rebuild the feature history", CadIcon::Rebuild,
        featureModel != nullptr, false, [&]{ rebuildFeaturesRequest_ = true; });
    ImGui::TextDisabled("|"); ImGui::SameLine();
    const bool editing = editor && editor->activeSketchFeatureId();
    commandButton("sketch", editing ? "Exit" : "Sketch", editing ? "Finish editing the active Sketch" : "Create or edit a Sketch",
        editing ? CadIcon::ExitSketch : CadIcon::Sketch, true, editing,
        [&]{ if (editing) exitSketchRequest_ = true; else { enterSketchRequest_ = true; propertyManagerVisible_ = true; } });
    const auto tool = [&](const char* id, const char* label, const char* tip, CadIcon icon,
                          SketchTool sketchTool, interaction::EditorCommand command)
    {
        commandButton(id, label, tip, icon, editing, activeTool == sketchTool,
            [&]{ sketchToolRequest_ = sketchTool; editorCommandRequest_ = command; propertyManagerVisible_ = true; });
    };
    tool("select", "Select", "Select sketch geometry", CadIcon::Select, SketchTool::Select, interaction::EditorCommand::Select);
    tool("line", "Line", "Create a two-point line", CadIcon::Line, SketchTool::Line, interaction::EditorCommand::Line);
    tool("rectangle", "Rectangle", "Create a corner rectangle", CadIcon::Rectangle, SketchTool::Rectangle, interaction::EditorCommand::Rectangle);
    tool("circle", "Circle", "Create a center-radius circle", CadIcon::Circle, SketchTool::Circle, interaction::EditorCommand::Circle);
    const bool selectedLine = selected && selected->type() == sketch::SketchEntityType::Line;
    commandButton("horizontal", "Horizontal", "Make the selected line horizontal", CadIcon::Horizontal,
        editing && selectedLine, false, [&]{
        editorCommandRequest_ = interaction::EditorCommand::Horizontal;
        addConstraintRequest_ = sketch::Horizontal{{selected->id(), sketch::SubElementKind::LineBody}};
    });
    commandButton("vertical", "Vertical", "Make the selected line vertical", CadIcon::Vertical,
        editing && selectedLine, false, [&]{
        editorCommandRequest_ = interaction::EditorCommand::Vertical;
        addConstraintRequest_ = sketch::Vertical{{selected->id(), sketch::SubElementKind::LineBody}};
    });
    commandButton("coincident", "Coincident", "Select two compatible endpoints", CadIcon::Coincident,
        editing, editor && editor->activeCommand()==interaction::EditorCommand::Coincident,
        [&]{ editorCommandRequest_ = interaction::EditorCommand::Coincident; propertyManagerVisible_ = true; });
    commandButton("dimension", "Dimension", "Create or edit a driving dimension", CadIcon::Dimension,
        editing && selected, editor && editor->activeCommand()==interaction::EditorCommand::Dimension,
        [&]{ editorCommandRequest_ = interaction::EditorCommand::Dimension; propertyManagerVisible_ = true; });
    ImGui::TextDisabled("|"); ImGui::SameLine();
    const sketch::Sketch* extrusionSketch = &model;
    if (editor && !editor->activeSketchFeatureId() && featureModel && selectedFeature_)
    {
        const auto& selectedFeature = featureModel->find(*selectedFeature_);
        if (selectedFeature.kind() == features::FeatureKind::Sketch)
            extrusionSketch = &std::get<features::SketchFeature>(selectedFeature.data()).sketch;
    }
    const auto regions = extrusionRegions(*extrusionSketch);
    const bool supportedProfile = regions.valid();
    const bool canExtrude = editor && editor->commandAvailable(interaction::EditorCommand::Extrude, supportedProfile);
    commandButton("extrude", "Extrude", supportedProfile ? "Create an extrusion from the active or selected Sketch"
        : regions.diagnostic.message.c_str(), CadIcon::Extrude,
        canExtrude, editor && editor->activeCommand()==interaction::EditorCommand::Extrude,
        [&]{ editorCommandRequest_ = interaction::EditorCommand::Extrude; propertyManagerVisible_ = true; });
    bool hasCutTarget=false;
    if(featureModel&&selectedFeature_&&featureModel->find(*selectedFeature_).kind()==features::FeatureKind::Sketch)
    {
        const auto& sketchFeature=std::get<features::SketchFeature>(featureModel->find(*selectedFeature_).data());
        hasCutTarget=std::holds_alternative<features::SemanticFaceSupport>(sketchFeature.support);
    }
    commandButton("cut-extrude", "Cut", hasCutTarget
        ? "Remove material normal to the selected face-supported Sketch"
        : "Select a closed Sketch created on an Extrude cap", CadIcon::CutExtrude,
        supportedProfile&&hasCutTarget,
        editor&&editor->activeCommand()==interaction::EditorCommand::CutExtrude,
        [&]{editorCommandRequest_=interaction::EditorCommand::CutExtrude;propertyManagerVisible_=true;});
    ImGui::End();
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
    if (ImGui::Selectable("Model", !propertyManagerVisible_, 0, ImVec2{145, 28}))
        propertyManagerVisible_ = false;
    ImGui::SameLine();
    if (ImGui::Selectable("Edit", propertyManagerVisible_, 0, ImVec2{145, 28}))
        propertyManagerVisible_ = true;
    ImGui::Separator();
    if (!propertyManagerVisible_)
    {
    ImGui::TextUnformatted("Document");
    ImGui::TextWrapped("%s%s", currentFilePath_.empty() ? "Untitled" : currentFilePath_.c_str(),
        documentDirty_ ? " *" : "");
    if (!persistenceStatus_.empty()) ImGui::TextWrapped("%s", persistenceStatus_.c_str());
    ImGui::Separator();
    if (featureModel)
    {
        const auto tree = ui::deriveFeatureTree(*featureModel, editor);
        ImGui::TextUnformatted("Feature Manager");
        ImGui::TextDisabled("How this part was built");
        const auto statusCue=[](const ui::FeatureTreeNode& node)
            -> std::pair<const char*,const char*>{
            if(node.activeEditing)return {"[*]","This sketch is being edited"};
            if(node.displayState==interaction::DisplayState::Hidden)return {"[-]","Hidden in the Graphics Area"};
            switch(node.state){
            case features::FeatureState::Dirty:return {"[~]","Needs rebuild"};
            case features::FeatureState::Failed:return {"[!]","This feature failed; inspect its message"};
            case features::FeatureState::BlockedByDependency:return {"[x]","Blocked by an earlier failed feature"};
            default:return {nullptr,nullptr};}
        };
        std::function<void(const ui::FeatureTreeNode&)> drawNode;
        drawNode=[&](const ui::FeatureTreeNode& node){
            ImGui::PushID(static_cast<int>(node.featureId.value()));
            auto flags=ImGuiTreeNodeFlags_SpanAvailWidth|ImGuiTreeNodeFlags_OpenOnArrow;
            if(selectedFeature_==node.featureId)flags|=ImGuiTreeNodeFlags_Selected;
            if(node.children.empty())flags|=ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
            const bool open=ImGui::TreeNodeEx(node.displayName.c_str(),flags);
            if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
                selectedFeature_=node.featureId;
                if(editor)editor->select(interaction::LogicalTarget{node.featureId});
            }
            if(ImGui::IsItemHovered()&&ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
                selectedFeature_=node.featureId;propertyManagerVisible_=true;
                if(node.kind==features::FeatureKind::Sketch)enterSketchRequest_=true;
            }
            if(ImGui::BeginPopupContextItem("Feature actions")){
                if(node.kind==features::FeatureKind::Sketch&&ImGui::MenuItem("Edit Sketch"))
                    {selectedFeature_=node.featureId;enterSketchRequest_=true;propertyManagerVisible_=true;}
                if(node.kind==features::FeatureKind::Sketch&&ImGui::MenuItem("Extrude"))
                    {selectedFeature_=node.featureId;editorCommandRequest_=interaction::EditorCommand::Extrude;propertyManagerVisible_=true;}
                if(node.kind==features::FeatureKind::Sketch&&ImGui::MenuItem("Cut Extrude"))
                    {selectedFeature_=node.featureId;editorCommandRequest_=interaction::EditorCommand::CutExtrude;propertyManagerVisible_=true;}
                if(node.kind==features::FeatureKind::Extrude&&ImGui::MenuItem("Edit"))
                    {selectedFeature_=node.featureId;propertyManagerVisible_=true;}
                if(node.kind==features::FeatureKind::CutExtrude&&ImGui::MenuItem("Edit"))
                    {selectedFeature_=node.featureId;propertyManagerVisible_=true;}
                if(node.kind==features::FeatureKind::Boolean&&ImGui::MenuItem("Inspect"))
                    {selectedFeature_=node.featureId;propertyManagerVisible_=true;}
                if(ImGui::MenuItem(node.displayState==interaction::DisplayState::Hidden?"Show":"Hide"))
                    visibilityToggleRequest_=node.featureId;
                ImGui::EndPopup();
            }
            if(const auto cue=statusCue(node);cue.first){
                ImGui::SameLine();ImGui::TextDisabled("%s",cue.first);
                if(ImGui::IsItemHovered()){ImGui::BeginTooltip();ImGui::TextUnformatted(cue.second);ImGui::EndTooltip();}
            }
            if(open&&!node.children.empty()){
                for(const auto& child:node.children)drawNode(child);
                ImGui::TreePop();
            }
            ImGui::PopID();
        };
        if(ImGui::TreeNodeEx(tree.rootName.c_str(),ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_SpanAvailWidth)){
            for(const auto& node:tree.roots)drawNode(node);
            ImGui::TreePop();
        }
        if (selectedFeature_)
        {
            const auto& feature = featureModel->find(*selectedFeature_);
            ImGui::Separator(); ImGui::Text("Selected: %s", feature.displayName().c_str());
            if(editor&&editor->selection()){
                if(std::holds_alternative<interaction::TopologyFaceTarget>(*editor->selection())){const auto&t=std::get<interaction::TopologyFaceTarget>(*editor->selection());ImGui::TextUnformatted("Face");const auto& cache=std::get<features::SolidFeatureCache>(*feature.cache());const auto& face=cache.solids.at(t.body.index).find(t.face);ImGui::TextUnformatted(std::holds_alternative<geometry::Plane>(face.support())?"Plane":"Cylinder");ImGui::Text("Owning feature: %s",feature.displayName().c_str());}
                else if(std::holds_alternative<interaction::TopologyEdgeTarget>(*editor->selection())){const auto&t=std::get<interaction::TopologyEdgeTarget>(*editor->selection());ImGui::TextUnformatted("Edge");const auto& cache=std::get<features::SolidFeatureCache>(*feature.cache());const auto& edge=cache.solids.at(t.body.index).find(t.edge);ImGui::TextUnformatted(std::holds_alternative<geometry::Segment3>(edge.geometry())?"Line":"Arc");ImGui::Text("Owning feature: %s",feature.displayName().c_str());}
            }
            if (feature.kind() == features::FeatureKind::Sketch)
            {
                const auto& sketch = std::get<features::SketchFeature>(feature.data()).sketch;
                ImGui::Text("%zu entities, %zu constraints", sketch.size(), sketch.constraintCount());
                if (ImGui::Button("Edit Sketch")) { enterSketchRequest_ = true; propertyManagerVisible_ = true; }
                ImGui::SameLine();
                if (ImGui::Button("Extrude")) { editorCommandRequest_ = interaction::EditorCommand::Extrude; propertyManagerVisible_ = true; }
                const bool faceSupported=std::holds_alternative<features::SemanticFaceSupport>(
                    std::get<features::SketchFeature>(feature.data()).support);
                if(faceSupported){ImGui::SameLine();if(ImGui::Button("Cut Extrude")){editorCommandRequest_=interaction::EditorCommand::CutExtrude;propertyManagerVisible_=true;}}
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
            else if(feature.kind()==features::FeatureKind::CutExtrude)
            {
                const auto&cut=std::get<features::CutExtrudeFeature>(feature.data());
                double depth=cut.distance;bool reverse=cut.direction==features::CutDirection::Reverse;
                ImGui::TextUnformatted("Property Manager — Cut Extrude");
                ImGui::Text("Profile: %s",featureModel->find(cut.sourceSketchFeatureId).displayName().c_str());
                ImGui::Text("Target: %s",featureModel->find(cut.targetResultFeatureId).displayName().c_str());
                ImGui::InputDouble("Depth",&depth,1.0,5.0,"%.3f");
                ImGui::Checkbox("Reverse Direction",&reverse);
                const bool valid=std::isfinite(depth)&&depth>0;if(!valid)ImGui::BeginDisabled();
                if(ImGui::Button("Confirm"))featureCutRequest_=std::tuple{feature.id(),depth,
                    reverse?features::CutDirection::Reverse:features::CutDirection::Forward};
                if(!valid)ImGui::EndDisabled();ImGui::SameLine();if(ImGui::Button("Cancel"))cancelPropertyRequest_=true;
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
        ImGui::TextWrapped("Select a feature to inspect it. Right-click a feature for Edit, Extrude, Show or Hide.");
        ImGui::End();
        return;
    }
    ImGui::TextUnformatted(editor && editor->activeSketchFeatureId() ? "Edit sketch" : "Edit model");
    const char* toolName = activeTool == SketchTool::Select ? "Select" : activeTool == SketchTool::Line ? "Line"
        : activeTool == SketchTool::Rectangle ? "Rectangle" : activeTool == SketchTool::Circle ? "Circle" : "Arc";
    ImGui::Text("Current tool: %s", toolName);
    if (editor && editor->activeSketchFeatureId())
    {
        ImGui::TextColored(ImVec4{0.35F, 0.75F, 1.0F, 1.0F},
            model.size() == 0 ? "What would you like to draw?" : "Your sketch is ready for the next step.");
        if (model.size() == 0)
        {
            ImGui::TextWrapped("Start with a closed rectangle. Choose a shape, then click in the Graphics Area.");
            if (ImGui::Button("Draw rectangle", ImVec2{180, 34}))
            {
                sketchToolRequest_ = SketchTool::Rectangle;
                editorCommandRequest_ = interaction::EditorCommand::Rectangle;
            }
            if (ImGui::Button("Draw circle", ImVec2{180, 30}))
            {
                sketchToolRequest_ = SketchTool::Circle;
                editorCommandRequest_ = interaction::EditorCommand::Circle;
            }
            ImGui::SameLine();
            if (ImGui::Button("Draw line", ImVec2{120, 30}))
            {
                sketchToolRequest_ = SketchTool::Line;
                editorCommandRequest_ = interaction::EditorCommand::Line;
            }
        }
        else
        {
            ImGui::TextWrapped("You have %zu sketch element%s.",
                model.size(), model.size() == 1 ? "" : "s");
            const auto regions = extrusionRegions(model);
            if (regions.valid())
            {
                ImGui::TextWrapped("%zu closed material region%s ready for 3D extrusion.",
                    regions.regions.size(), regions.regions.size() == 1 ? " is" : "s are");
                if (ImGui::Button("Continue to 3D extrusion", ImVec2{230, 36}))
                {
                    editorCommandRequest_ = interaction::EditorCommand::Extrude;
                    propertyManagerVisible_ = true;
                }
            }
            else
                ImGui::TextColored(ImVec4{1.0F,0.72F,0.25F,1.0F},
                    "%s", regions.diagnostic.message.c_str());
            if (ImGui::Button("Keep drawing", ImVec2{115, 36}))
            {
                sketchToolRequest_ = SketchTool::Rectangle;
                editorCommandRequest_ = interaction::EditorCommand::Rectangle;
            }
        }
        ImGui::Spacing();
        ImGui::Separator();
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

    if (editor && editor->activeCommand() == interaction::EditorCommand::Extrude)
    {
        ImGui::TextUnformatted("Create a 3D extrusion");
        ImGui::Spacing();
        ImGui::TextWrapped("1. Source: the selected sketch");
        ImGui::TextWrapped("2. Enter how far the shape should extend");
        ImGui::TextWrapped("3. Choose Create extrusion");
        ImGui::Spacing();
        ImGui::InputDouble("Depth", &extrusionDistance_, 1.0, 5.0, "%.3f");
        const auto regions = extrusionRegions(model);
        const bool hasProfile = regions.valid();
        const bool hasPositiveDepth = std::isfinite(extrusionDistance_) && extrusionDistance_ > 0;
        if (!hasProfile)
            ImGui::TextColored(ImVec4{1,0.35F,0.25F,1}, "%s",
                regions.diagnostic.message.c_str());
        else
        {
            ImGui::TextColored(ImVec4{0.35F,0.9F,0.45F,1},
                "%zu region%s will create %zu solid bod%s.", regions.regions.size(),
                regions.regions.size() == 1 ? "" : "s", regions.regions.size(),
                regions.regions.size() == 1 ? "y" : "ies");
        }
        if (!hasPositiveDepth)
            ImGui::TextColored(ImVec4{1,0.35F,0.25F,1}, "Depth must be greater than zero.");
        const bool valid = hasProfile && hasPositiveDepth;
        if (!valid) ImGui::BeginDisabled();
        if (ImGui::Button("Create extrusion", ImVec2{170, 34}))
        {
            extrusionRequest_ = true;
            confirmPropertyRequest_ = true;
        }
        if (!valid) ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2{90, 34})) cancelPropertyRequest_ = true;
        if (!extrusionStatus_.empty()) ImGui::TextWrapped("%s", extrusionStatus_.c_str());
        ImGui::End();
        return;
    }

    if(editor&&editor->activeCommand()==interaction::EditorCommand::CutExtrude)
    {
        ImGui::TextUnformatted("Create a Cut Extrude");ImGui::Spacing();
        std::optional<features::FeatureId>target;
        if(featureModel&&selectedFeature_&&featureModel->find(*selectedFeature_).kind()==features::FeatureKind::Sketch)
        {
            const auto&source=std::get<features::SketchFeature>(featureModel->find(*selectedFeature_).data());
            if(const auto*support=std::get_if<features::SemanticFaceSupport>(&source.support))target=support->sourceFeatureId;
        }
        ImGui::Text("Profile: %s",selectedFeature_&&featureModel?featureModel->find(*selectedFeature_).displayName().c_str():"Select a Sketch");
        ImGui::Text("Target: %s",target&&featureModel?featureModel->find(*target).displayName().c_str():"Unavailable");
        ImGui::InputDouble("Depth",&cutDepth_,1.0,5.0,"%.3f");
        ImGui::Checkbox("Reverse Direction",&cutReverseDirection_);
        const auto regions=extrusionRegions(model);const bool valid=regions.valid()&&target
            &&std::isfinite(cutDepth_)&&cutDepth_>0;
        if(!regions.valid())ImGui::TextColored(ImVec4{1,0.35F,0.25F,1},"%s",regions.diagnostic.message.c_str());
        if(!target)ImGui::TextColored(ImVec4{1,0.35F,0.25F,1},"Create this Sketch on an Extrude start or end face.");
        if(!valid)ImGui::BeginDisabled();
        if(ImGui::Button("Create cut",ImVec2{170,34}))cutExtrudeRequest_=CutExtrudeRequest{
            *selectedFeature_,*target,cutDepth_,cutReverseDirection_?features::CutDirection::Reverse:features::CutDirection::Forward};
        if(!valid)ImGui::EndDisabled();ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2{90,34}))cancelPropertyRequest_=true;
        ImGui::TextWrapped("The target is inferred from the supporting face. No internal IDs are required.");
        ImGui::End();return;
    }

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
            exitDocumentRequest_ = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, canUndo_)) undoRequest_ = true;
        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, canRedo_)) redoRequest_ = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        if(ImGui::MenuItem("Fit"))viewCommandRequest_=ViewCommand::Fit;
        if(ImGui::BeginMenu("Standard Views")){
            if(ImGui::MenuItem("Front"))viewCommandRequest_=ViewCommand::Front;if(ImGui::MenuItem("Back"))viewCommandRequest_=ViewCommand::Back;
            if(ImGui::MenuItem("Left"))viewCommandRequest_=ViewCommand::Left;if(ImGui::MenuItem("Right"))viewCommandRequest_=ViewCommand::Right;
            if(ImGui::MenuItem("Top"))viewCommandRequest_=ViewCommand::Top;if(ImGui::MenuItem("Bottom"))viewCommandRequest_=ViewCommand::Bottom;
            if(ImGui::MenuItem("Isometric"))viewCommandRequest_=ViewCommand::Isometric;ImGui::EndMenu();}
    if(ImGui::BeginMenu("Display Mode")){
        const auto mode=[&](const char* label,DisplayModeRequest value){if(ImGui::MenuItem(label,nullptr,displayMode_==value)){displayMode_=value;displayModeRequest_=value;}};
        mode("Shaded",DisplayModeRequest::Shaded);mode("Shaded With Edges",DisplayModeRequest::ShadedWithEdges);mode("Wireframe",DisplayModeRequest::Wireframe);ImGui::EndMenu();}
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

void ApplicationShell::drawWorkspace(SketchTool activeTool, const interaction::EditorContext* editor)
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
    ImGui::SameLine();ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(),size.x-235));
    if(ImGui::SmallButton("Fit"))viewCommandRequest_=ViewCommand::Fit;if(ImGui::IsItemHovered())ImGui::SetTooltip("Fit visible modeled geometry");
    ImGui::SameLine();if(ImGui::SmallButton("ISO"))viewCommandRequest_=ViewCommand::Isometric;if(ImGui::IsItemHovered())ImGui::SetTooltip("Isometric view");
    ImGui::SameLine();if(displayModeButton(displayMode_))ImGui::OpenPopup("Display mode");
    if(ImGui::BeginPopup("Display mode")){
        const auto mode=[&](const char* label,DisplayModeRequest value){if(ImGui::MenuItem(label,nullptr,displayMode_==value)){displayMode_=value;displayModeRequest_=value;}};
        mode("Shaded",DisplayModeRequest::Shaded);mode("Shaded With Edges",DisplayModeRequest::ShadedWithEdges);mode("Wireframe",DisplayModeRequest::Wireframe);ImGui::EndPopup();}
    input_.workspaceHovered = ImGui::IsWindowHovered();
    if (input_.workspaceHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::OpenPopup("Graphics context");
    if (ImGui::BeginPopup("Graphics context"))
    {
        if (selectedFeature_)
        {
            if (ImGui::MenuItem("Edit selected Sketch")) { enterSketchRequest_ = true; propertyManagerVisible_ = true; }
            if (ImGui::MenuItem("Show / Hide")) visibilityToggleRequest_ = selectedFeature_;
            if (ImGui::MenuItem("Extrude")) { editorCommandRequest_ = interaction::EditorCommand::Extrude; propertyManagerVisible_ = true; }
        }
        else ImGui::TextDisabled("No logical object selected");
        if (ImGui::MenuItem("Rebuild")) rebuildFeaturesRequest_ = true;
        ImGui::EndPopup();
    }
    if (editor)
    {
        auto* draw = ImGui::GetForegroundDrawList();
        const auto toScreen = [&](interaction::ScreenPoint point) {
            return ImVec2{viewport->Pos.x + static_cast<float>(point.x),
                          viewport->Pos.y + static_cast<float>(point.y)};
        };
        if (editor->previewAnchor() && editor->previewCursor())
        {
            const auto anchor = toScreen(*editor->previewAnchor());
            const auto cursor = toScreen(*editor->previewCursor());
            const auto previewColor = IM_COL32(80, 220, 255, 220);
            const auto& curve = editor->previewCurve();
            if (curve.size() > 1)
            {
                for (std::size_t i = 1; i < curve.size(); ++i)
                    draw->AddLine(toScreen(curve[i - 1]), toScreen(curve[i]), previewColor, 2.0F);
                if (activeTool == SketchTool::Rectangle || activeTool == SketchTool::Circle)
                    draw->AddLine(toScreen(curve.back()), toScreen(curve.front()), previewColor, 2.0F);
            }
            else
                draw->AddLine(anchor, cursor, previewColor, 2.0F);
        }
        const auto cursor = ImGui::GetMousePos();
        if (editor->snapCandidate())
        {
            const auto snapColor = IM_COL32(255, 210, 60, 255);
            if (editor->snapCandidate()->kind == interaction::SnapKind::Endpoint)
                draw->AddRect(ImVec2{cursor.x - 5, cursor.y - 5}, ImVec2{cursor.x + 5, cursor.y + 5}, snapColor, 0, 0, 2);
            else if (editor->snapCandidate()->kind == interaction::SnapKind::Midpoint)
                draw->AddTriangle(ImVec2{cursor.x, cursor.y - 6}, ImVec2{cursor.x - 6, cursor.y + 5}, ImVec2{cursor.x + 6, cursor.y + 5}, snapColor, 2);
            else draw->AddCircle(cursor, 5, snapColor, 12, 2);
        }
        if (editor->inferenceCandidate())
        {
            const char* cue = editor->inferenceCandidate()->kind == interaction::InferenceKind::Horizontal ? "H"
                : editor->inferenceCandidate()->kind == interaction::InferenceKind::Vertical ? "V" : "C";
            draw->AddText(ImVec2{cursor.x + 9, cursor.y + 7}, IM_COL32(100, 255, 120, 255), cue);
        }
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
    const std::string documentLabel = (currentFilePath_.empty() ? "Untitled" : currentFilePath_) + (documentDirty_ ? " *" : "");
    ImGui::Text("%s | %s", documentLabel.c_str(),
        persistenceStatus_.empty() ? "Ready" : persistenceStatus_.c_str());
    ImGui::End();
}

void ApplicationShell::setDocumentState(bool dirty, bool canUndo, bool canRedo)
{
    documentDirty_ = dirty; canUndo_ = canUndo; canRedo_ = canRedo;
}

void ApplicationShell::showUnsavedChangesDialog() { unsavedDialogRequested_ = true; }

void ApplicationShell::drawUnsavedChangesDialog()
{
    if (unsavedDialogRequested_) { ImGui::OpenPopup("Unsaved changes"); unsavedDialogRequested_ = false; }
    if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("You have unsaved changes.");
        ImGui::Text("Save changes to %s?", currentFilePath_.empty() ? "Untitled" : currentFilePath_.c_str());
        ImGui::TextDisabled("Your changes will be lost if you don't save them.");
        if (ImGui::Button("Save")) { unsavedDecision_ = UnsavedDecision::Save; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Don't Save")) { unsavedDecision_ = UnsavedDecision::DontSave; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { unsavedDecision_ = UnsavedDecision::Cancel; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
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
