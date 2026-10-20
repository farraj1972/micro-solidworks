#pragma once

#include "app/WorkspaceLayout.h"
#include "app/WorkspaceInput.h"
#include "presentation/VisualEntity.h"
#include "app/sketch/SketchToolController.h"
#include "modeling/Boolean.h"
#include "features/FeatureModel.h"
#include "interaction/EditorContext.h"
#include "app/preferences/ApplicationPreferences.h"
#include "export/ExportMesh.h"

#include <optional>
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <tuple>
#include <unordered_map>

namespace microsw
{
enum class ViewCommand { Fit, Front, Back, Left, Right, Top, Bottom, Isometric };
enum class PrincipalPlane { Front, Top, Right };
enum class DisplayModeRequest { ShadedWithEdges, Wireframe, Shaded };
enum class UnsavedDecision { Save, DontSave, Cancel };
struct CutExtrudeRequest
{
    features::FeatureId sketch;
    features::FeatureId target;
    math::Scalar depth;
    features::CutDirection direction;
};
struct CenteredRectangleRequest
{
    geometry::Point2 center;
    math::Scalar width;
    math::Scalar height;
};
struct PreferencesApplyRequest
{
    preferences::ApplicationPreferences candidate;
    bool closeOnSuccess{};
    bool confirmFutureVersionReset{};
};
class ApplicationWindow;

class ApplicationShell
{
public:
    explicit ApplicationShell(ApplicationWindow& window);

    void draw(ProjectionMode projectionMode, const presentation::VisualEntity* selected = nullptr);
    void drawSketch(ProjectionMode projectionMode, SketchTool activeTool,
                    const sketch::Sketch& sketch, const sketch::SketchEntity* selected = nullptr,
                    bool hasActiveSolid = false, const features::FeatureModel* featureModel = nullptr,
                    interaction::EditorContext* editor = nullptr);
    [[nodiscard]] const std::optional<math::Transform3>& transformRequest() const noexcept { return transformRequest_; }
    void reportTransformError(const std::string& error) { transformError_ = error; }
    [[nodiscard]] const std::optional<SketchTool>& sketchToolRequest() const noexcept { return sketchToolRequest_; }
    [[nodiscard]] const std::optional<CenteredRectangleRequest>& centeredRectangleRequest() const noexcept { return centeredRectangleRequest_; }
    [[nodiscard]] const std::optional<sketch::SketchGeometry>& sketchGeometryRequest() const noexcept { return sketchGeometryRequest_; }
    [[nodiscard]] bool deleteSketchRequest() const noexcept { return deleteSketchRequest_; }
    [[nodiscard]] const std::optional<sketch::SketchConstraintValue>& addConstraintRequest() const noexcept { return addConstraintRequest_; }
    [[nodiscard]] const std::optional<sketch::SketchConstraintId>& removeConstraintRequest() const noexcept { return removeConstraintRequest_; }
    [[nodiscard]] const std::optional<std::pair<sketch::SketchConstraintId, math::Scalar>>& drivingValueRequest() const noexcept { return drivingValueRequest_; }
    [[nodiscard]] bool solveSketchRequest() const noexcept { return solveSketchRequest_; }
    void reportSketchError(const std::string& error) { sketchError_ = error; workflowFeedback_ = error; }
    [[nodiscard]] bool extrusionRequest() const noexcept { return extrusionRequest_; }
    [[nodiscard]] math::Scalar extrusionDistance() const noexcept { return extrusionDistance_; }
    [[nodiscard]] const std::optional<CutExtrudeRequest>& cutExtrudeRequest() const noexcept { return cutExtrudeRequest_; }
    [[nodiscard]] const std::optional<std::tuple<features::FeatureId, math::Scalar, features::CutDirection>>& featureCutRequest() const noexcept { return featureCutRequest_; }
    [[nodiscard]] math::Scalar cutDepth() const noexcept { return cutDepth_; }
    void reportExtrusionError(const std::string& error) { extrusionStatus_ = "Extrusion rejected: " + error; workflowFeedback_ = extrusionStatus_; }
    void reportExtrusionSuccess() { extrusionStatus_ = "Solid regenerated"; workflowFeedback_ = extrusionStatus_; }
    [[nodiscard]] bool loadBooleanDemoRequest() const noexcept { return loadBooleanDemoRequest_; }
    [[nodiscard]] bool loadBooleanUnsupportedRequest() const noexcept { return loadBooleanUnsupportedRequest_; }
    [[nodiscard]] bool assignOperandARequest() const noexcept { return assignOperandARequest_; }
    [[nodiscard]] bool assignOperandBRequest() const noexcept { return assignOperandBRequest_; }
    [[nodiscard]] const std::optional<modeling::BooleanOperation>& booleanRequest() const noexcept { return booleanRequest_; }
    void reportBooleanStatus(modeling::BooleanStatus status);
    void reportBooleanOperandsReady() { booleanStatus_ = "Operands A/B ready"; }
    [[nodiscard]] const std::optional<std::pair<features::FeatureId, math::Scalar>>& featureDistanceRequest() const noexcept { return featureDistanceRequest_; }
    [[nodiscard]] const std::optional<std::pair<features::FeatureId, modeling::BooleanOperation>>& featureBooleanRequest() const noexcept { return featureBooleanRequest_; }
    [[nodiscard]] bool rebuildFeaturesRequest() const noexcept { return rebuildFeaturesRequest_; }
    void reportFeatureRebuild(const features::RebuildResult& result,
                              const features::FeatureModel& model);
    [[nodiscard]] bool newDocumentRequest() const noexcept { return newDocumentRequest_; }
    [[nodiscard]] bool openDocumentRequest() const noexcept { return openDocumentRequest_; }
    [[nodiscard]] bool saveDocumentRequest() const noexcept { return saveDocumentRequest_; }
    [[nodiscard]] bool saveAsDocumentRequest() const noexcept { return saveAsDocumentRequest_; }
    [[nodiscard]] bool exportBrowseRequest() const noexcept { return exportBrowseRequest_; }
    [[nodiscard]] bool exportValidateRequest() const noexcept { return exportValidateRequest_; }
    [[nodiscard]] bool exportWriteRequest() const noexcept { return exportWriteRequest_; }
    [[nodiscard]] std::size_t exportBodyIndex() const noexcept { return exportBodyIndex_; }
    [[nodiscard]] exporting::MeshQuality exportQuality() const noexcept;
    [[nodiscard]] const std::filesystem::path& exportPath() const noexcept { return exportPath_; }
    void setExportPath(std::filesystem::path path) { exportPath_ = std::move(path); exportValidated_ = false; }
    void reportExportValidation(bool success, std::string diagnostic)
    { exportValidated_ = success; exportStatus_ = std::move(diagnostic); workflowFeedback_ = exportStatus_; }
    void reportExportWrite(bool success, std::string diagnostic)
    { exportStatus_ = std::move(diagnostic); workflowFeedback_ = exportStatus_; if (!success) exportValidated_ = false; }
    [[nodiscard]] bool undoRequest() const noexcept { return undoRequest_; }
    [[nodiscard]] bool redoRequest() const noexcept { return redoRequest_; }
    [[nodiscard]] bool exitDocumentRequest() const noexcept { return exitDocumentRequest_; }
    void setDocumentState(bool dirty, bool canUndo, bool canRedo);
    void showUnsavedChangesDialog();
    [[nodiscard]] const std::optional<UnsavedDecision>& unsavedDecision() const noexcept { return unsavedDecision_; }
    void reportPersistenceStatus(std::string status) { persistenceStatus_ = std::move(status); workflowFeedback_ = persistenceStatus_; }
    void setCurrentFilePath(std::string path) { currentFilePath_ = std::move(path); }
    void setPreferences(preferences::ApplicationPreferences current, bool futureVersionNeedsReset = false);
    [[nodiscard]] const std::optional<PreferencesApplyRequest>& preferencesApplyRequest() const noexcept
    { return preferencesApplyRequest_; }
    void reportPreferencesResult(bool success, std::string diagnostic);
    void reportPreferencesLoad(std::string diagnostic) { preferencesDiagnostic_ = std::move(diagnostic); }
    [[nodiscard]] const WorkspaceLayout& workspaceRect() const noexcept { return workspace_; }
    [[nodiscard]] const WorkspaceInput& workspaceInput() const noexcept { return input_; }
    [[nodiscard]] const std::optional<interaction::EditorCommand>& editorCommandRequest() const noexcept { return editorCommandRequest_; }
    [[nodiscard]] const std::optional<features::FeatureId>& selectedFeature() const noexcept { return selectedFeature_; }
    void setSelectedFeature(std::optional<features::FeatureId> feature)
    {
        if (feature != selectedFeature_)
        {
            editingFeature_.reset();
            renamingFeature_.reset();
            featureEditorEntity_.reset();
        }
        selectedFeature_ = feature;
        if (feature) selectedPrincipalPlane_.reset();
    }
    [[nodiscard]] bool enterSketchRequest() const noexcept { return enterSketchRequest_; }
    [[nodiscard]] const std::optional<PrincipalPlane>& sketchPlaneRequest() const noexcept { return sketchPlaneRequest_; }
    [[nodiscard]] bool exitSketchRequest() const noexcept { return exitSketchRequest_; }
    [[nodiscard]] bool confirmPropertyRequest() const noexcept { return confirmPropertyRequest_; }
    [[nodiscard]] bool cancelPropertyRequest() const noexcept { return cancelPropertyRequest_; }
    [[nodiscard]] bool gridSnapEnabled() const noexcept { return gridSnapEnabled_; }
    [[nodiscard]] const std::optional<features::FeatureId>& visibilityToggleRequest() const noexcept { return visibilityToggleRequest_; }
    [[nodiscard]] const std::optional<features::FeatureId>& deleteFeatureRequest() const noexcept { return deleteFeatureRequest_; }
    [[nodiscard]] const std::optional<std::pair<features::FeatureId, std::string>>& renameFeatureRequest() const noexcept { return renameFeatureRequest_; }
    [[nodiscard]] const std::optional<ViewCommand>& viewCommandRequest() const noexcept { return viewCommandRequest_; }
    [[nodiscard]] const std::optional<DisplayModeRequest>& displayModeRequest() const noexcept { return displayModeRequest_; }

private:
    void drawDocumentHeader();
    void drawMainMenu(ProjectionMode projectionMode);
    void drawModelPanel(const presentation::VisualEntity* selected);
    void drawSketchPanel(SketchTool activeTool, const sketch::Sketch& sketch,
                         const sketch::SketchEntity* selected, bool hasActiveSolid,
                         const features::FeatureModel* featureModel,
                         interaction::EditorContext* editor);
    void drawCommandBar(SketchTool activeTool, const sketch::Sketch& sketch,
                        const sketch::SketchEntity* selected,
                        const features::FeatureModel* featureModel,
                        interaction::EditorContext* editor);
    void drawWorkspace(SketchTool activeTool = SketchTool::Select,
                       const interaction::EditorContext* editor = nullptr,
                       const features::FeatureModel* featureModel = nullptr);
    void drawStatusBar();
    void drawAboutDialog();
    void drawUnsavedChangesDialog();
    void drawPreferencesDialog();
    void drawExportDialog(const features::FeatureModel* model);
    void beginFeatureEdit(features::FeatureId id, const features::FeatureModel& model);
    void beginFeatureRename(features::FeatureId id, const features::FeatureModel& model);

    ApplicationWindow& window_;
    WorkspaceLayout workspace_{};
    WorkspaceInput input_{};
    bool aboutDialogRequested_{false};
    bool preferencesDialogRequested_{};
    bool preferencesDialogVisible_{};
    bool preferencesCloseRequested_{};
    bool preferencesFutureVersion_{};
    bool preferencesFutureResetConfirmed_{};
    bool preferencesPendingClose_{};
    preferences::ApplicationPreferences preferencesActive_{};
    preferences::ApplicationPreferences preferencesDraft_{};
    std::optional<PreferencesApplyRequest> preferencesApplyRequest_;
    std::string preferencesDiagnostic_;
    std::optional<math::Transform3> transformRequest_;
    std::optional<presentation::VisualEntityId> editorEntity_;
    std::string transformError_;
    std::optional<SketchTool> sketchToolRequest_;
    std::optional<CenteredRectangleRequest> centeredRectangleRequest_;
    std::array<double, 4> centeredRectangleDraft_{0.0, 0.0, 10.0, 10.0};
    std::optional<sketch::SketchGeometry> sketchGeometryRequest_;
    bool deleteSketchRequest_{};
    std::optional<sketch::SketchEntityId> sketchEditorEntity_;
    std::array<double, 5> sketchGeometryDraft_{};
    bool sketchGeometryDraftDirty_{};
    struct DimensionDraft { math::Scalar value{}, committedValue{}; };
    std::optional<features::FeatureId> sketchDimensionDraftFeature_;
    std::unordered_map<std::uint64_t, DimensionDraft> sketchDimensionDrafts_;
    std::string sketchError_;
    std::optional<sketch::SketchConstraintValue> addConstraintRequest_;
    std::optional<sketch::SketchConstraintId> removeConstraintRequest_;
    std::optional<std::pair<sketch::SketchConstraintId, math::Scalar>> drivingValueRequest_;
    bool solveSketchRequest_{};
    bool extrusionRequest_{};
    math::Scalar extrusionDistance_{5.0};
    math::Scalar cutDepth_{5.0};
    bool cutReverseDirection_{true};
    std::optional<CutExtrudeRequest> cutExtrudeRequest_;
    std::string extrusionStatus_;
    bool loadBooleanDemoRequest_{};
    bool loadBooleanUnsupportedRequest_{};
    bool assignOperandARequest_{};
    bool assignOperandBRequest_{};
    std::optional<modeling::BooleanOperation> booleanRequest_;
    std::string booleanStatus_;
    std::optional<features::FeatureId> selectedFeature_;
    std::optional<PrincipalPlane> selectedPrincipalPlane_;
    std::optional<PrincipalPlane> sketchPlaneRequest_;
    std::optional<std::pair<features::FeatureId, math::Scalar>> featureDistanceRequest_;
    std::optional<std::pair<features::FeatureId, modeling::BooleanOperation>> featureBooleanRequest_;
    std::optional<std::tuple<features::FeatureId, math::Scalar, features::CutDirection>> featureCutRequest_;
    std::optional<features::FeatureId> featureEditorEntity_;
    std::optional<features::FeatureId> editingFeature_;
    double featureDepthDraft_{};
    bool featureReverseDraft_{};
    bool rebuildFeaturesRequest_{};
    std::string featureRebuildStatus_;
    bool newDocumentRequest_{};
    bool openDocumentRequest_{};
    bool saveDocumentRequest_{};
    bool saveAsDocumentRequest_{};
    bool exportDialogRequested_{};
    bool exportBrowseRequest_{};
    bool exportValidateRequest_{};
    bool exportWriteRequest_{};
    std::size_t exportBodyIndex_{};
    bool exportFineQuality_{};
    bool exportValidated_{};
    std::filesystem::path exportPath_;
    std::string exportStatus_;
    bool undoRequest_{};
    bool redoRequest_{};
    bool exitDocumentRequest_{};
    bool documentDirty_{};
    bool canUndo_{};
    bool canRedo_{};
    bool unsavedDialogRequested_{};
    std::optional<UnsavedDecision> unsavedDecision_;
    std::optional<interaction::EditorCommand> editorCommandRequest_;
    bool enterSketchRequest_{};
    bool exitSketchRequest_{};
    bool confirmPropertyRequest_{};
    bool cancelPropertyRequest_{};
    bool gridSnapEnabled_{true};
    std::optional<features::FeatureId> visibilityToggleRequest_;
    std::optional<features::FeatureId> deleteFeatureRequest_;
    std::optional<std::pair<features::FeatureId, std::string>> renameFeatureRequest_;
    std::optional<features::FeatureId> renamingFeature_;
    std::array<char, 128> renameDraft_{};
    std::string renameError_;
    bool openRenamePopup_{};
    std::optional<features::FeatureId> pendingDeleteFeature_;
    bool openDeleteConfirmation_{};
    std::optional<ViewCommand> viewCommandRequest_;
    std::optional<DisplayModeRequest> displayModeRequest_;
    DisplayModeRequest displayMode_{DisplayModeRequest::ShadedWithEdges};
    std::string currentFilePath_;
    std::string persistenceStatus_;
    std::string workflowState_{"Part editing"};
    std::string workflowGuidance_{"Select a feature or reference plane to begin."};
    std::string workflowFeedback_;
    float modelPanelWidth_{320.0F};
    float inspectorPanelWidth_{300.0F};
};
}
