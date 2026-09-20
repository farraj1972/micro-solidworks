#pragma once

#include "app/WorkspaceLayout.h"
#include "app/WorkspaceInput.h"
#include "presentation/VisualEntity.h"
#include "app/sketch/SketchToolController.h"

#include <optional>
#include <string>

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
                    bool hasActiveSolid = false);
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
    [[nodiscard]] const WorkspaceLayout& workspaceRect() const noexcept { return workspace_; }
    [[nodiscard]] const WorkspaceInput& workspaceInput() const noexcept { return input_; }

private:
    void drawMainMenu(ProjectionMode projectionMode);
    void drawModelPanel(const presentation::VisualEntity* selected);
    void drawSketchPanel(SketchTool activeTool, const sketch::Sketch& sketch,
                         const sketch::SketchEntity* selected, bool hasActiveSolid);
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
    float modelPanelWidth_{320.0F};
};
}
