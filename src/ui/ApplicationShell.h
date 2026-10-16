#pragma once

#include "app/WorkspaceLayout.h"
#include "app/WorkspaceInput.h"
#include "presentation/VisualEntity.h"
#include "app/sketch/SketchToolController.h"
#include "modeling/Boolean.h"
#include "features/FeatureModel.h"

#include <optional>
#include <string>
#include <array>

namespace microsw
{
class ApplicationWindow;

class ApplicationShell
{
public:
    explicit ApplicationShell(ApplicationWindow& window);

    void draw(ProjectionMode projectionMode, const presentation::VisualEntity* selected = nullptr);
    void drawSketch(ProjectionMode projectionMode, SketchTool activeTool,
                    const sketch::Sketch& sketch, const sketch::SketchEntity* selected = nullptr,
                    bool hasActiveSolid = false, const features::FeatureModel* featureModel = nullptr);
    [[nodiscard]] const std::optional<math::Transform3>& transformRequest() const noexcept { return transformRequest_; }
    void reportTransformError(const std::string& error) { transformError_ = error; }
    [[nodiscard]] const std::optional<SketchTool>& sketchToolRequest() const noexcept { return sketchToolRequest_; }
    [[nodiscard]] const std::optional<sketch::SketchGeometry>& sketchGeometryRequest() const noexcept { return sketchGeometryRequest_; }
    [[nodiscard]] bool deleteSketchRequest() const noexcept { return deleteSketchRequest_; }
    [[nodiscard]] const std::optional<sketch::SketchConstraintValue>& addConstraintRequest() const noexcept { return addConstraintRequest_; }
    [[nodiscard]] const std::optional<sketch::SketchConstraintId>& removeConstraintRequest() const noexcept { return removeConstraintRequest_; }
    [[nodiscard]] const std::optional<std::pair<sketch::SketchConstraintId, math::Scalar>>& drivingValueRequest() const noexcept { return drivingValueRequest_; }
    [[nodiscard]] bool solveSketchRequest() const noexcept { return solveSketchRequest_; }
    void reportSketchError(const std::string& error) { sketchError_ = error; }
    [[nodiscard]] bool extrusionRequest() const noexcept { return extrusionRequest_; }
    [[nodiscard]] math::Scalar extrusionDistance() const noexcept { return extrusionDistance_; }
    void reportExtrusionError(const std::string& error) { extrusionStatus_ = "Extrusion rejected: " + error; }
    void reportExtrusionSuccess() { extrusionStatus_ = "Solid regenerated"; }
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
    void reportFeatureRebuild(const features::RebuildResult& result);
    [[nodiscard]] bool newDocumentRequest() const noexcept { return newDocumentRequest_; }
    [[nodiscard]] bool openDocumentRequest() const noexcept { return openDocumentRequest_; }
    [[nodiscard]] bool saveDocumentRequest() const noexcept { return saveDocumentRequest_; }
    [[nodiscard]] bool saveAsDocumentRequest() const noexcept { return saveAsDocumentRequest_; }
    [[nodiscard]] std::string documentPathInput() const { return documentPathInput_.data(); }
    void reportPersistenceStatus(std::string status) { persistenceStatus_ = std::move(status); }
    void setCurrentFilePath(std::string path) { currentFilePath_ = std::move(path); }
    [[nodiscard]] const WorkspaceLayout& workspaceRect() const noexcept { return workspace_; }
    [[nodiscard]] const WorkspaceInput& workspaceInput() const noexcept { return input_; }

private:
    void drawMainMenu(ProjectionMode projectionMode);
    void drawModelPanel(const presentation::VisualEntity* selected);
    void drawSketchPanel(SketchTool activeTool, const sketch::Sketch& sketch,
                         const sketch::SketchEntity* selected, bool hasActiveSolid,
                         const features::FeatureModel* featureModel);
    void drawWorkspace();
    void drawStatusBar();
    void drawAboutDialog();

    ApplicationWindow& window_;
    WorkspaceLayout workspace_{};
    WorkspaceInput input_{};
    bool aboutDialogRequested_{false};
    std::optional<math::Transform3> transformRequest_;
    std::optional<presentation::VisualEntityId> editorEntity_;
    std::string transformError_;
    std::optional<SketchTool> sketchToolRequest_;
    std::optional<sketch::SketchGeometry> sketchGeometryRequest_;
    bool deleteSketchRequest_{};
    std::optional<sketch::SketchEntityId> sketchEditorEntity_;
    std::string sketchError_;
    std::optional<sketch::SketchConstraintValue> addConstraintRequest_;
    std::optional<sketch::SketchConstraintId> removeConstraintRequest_;
    std::optional<std::pair<sketch::SketchConstraintId, math::Scalar>> drivingValueRequest_;
    bool solveSketchRequest_{};
    bool extrusionRequest_{};
    math::Scalar extrusionDistance_{5.0};
    std::string extrusionStatus_;
    bool loadBooleanDemoRequest_{};
    bool loadBooleanUnsupportedRequest_{};
    bool assignOperandARequest_{};
    bool assignOperandBRequest_{};
    std::optional<modeling::BooleanOperation> booleanRequest_;
    std::string booleanStatus_;
    std::optional<features::FeatureId> selectedFeature_;
    std::optional<std::pair<features::FeatureId, math::Scalar>> featureDistanceRequest_;
    std::optional<std::pair<features::FeatureId, modeling::BooleanOperation>> featureBooleanRequest_;
    bool rebuildFeaturesRequest_{};
    std::string featureRebuildStatus_;
    bool newDocumentRequest_{};
    bool openDocumentRequest_{};
    bool saveDocumentRequest_{};
    bool saveAsDocumentRequest_{};
    std::array<char, 512> documentPathInput_{};
    std::string currentFilePath_;
    std::string persistenceStatus_;
    float modelPanelWidth_{320.0F};
};
}
