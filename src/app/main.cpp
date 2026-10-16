#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "app/sketch/SketchToolController.h"
#include "presentation/SketchPresentation.h"
#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "viewer/WorkspaceViewport.h"
#include "constraints/ConstraintSolver.h"
#include "app/modeling/ActiveExtrusion.h"
#include "app/modeling/ActiveBoolean.h"
#include "modeling/Extrusion.h"
#include "core/geometry/GeometricTolerance.h"
#include "features/FeatureModel.h"
#include "presentation/FeaturePresentation.h"
#include "app/PersistenceSession.h"

#include <exception>
#include <stdexcept>

namespace
{
microsw::constraints::SolverPolicy modelingSolverPolicy()
{
    microsw::constraints::SolverPolicy policy;
    policy.residualSatisfactionTolerance = microsw::geometry::defaultGeometricTolerance;
    policy.stepTolerance = microsw::geometry::defaultGeometricTolerance * 0.01;
    policy.costReductionTolerance = 1e-16;
    policy.initialStateRegularization = 1e-12;
    policy.maxIterations = 200;
    return policy;
}
}

int main()
{
    microsw::Logger::info("Application starting");

    try
    {
        {
            microsw::ApplicationWindow window{1280, 720, "Micro SolidWorks"};
            microsw::OpenGLContext openGLContext{window};
            microsw::ImGuiLayer ui{window, openGLContext};
            microsw::ApplicationShell shell{window};
            microsw::sketch::Sketch sketch;
            microsw::SketchToolController tools{sketch};
            microsw::presentation::SketchPresentation presentation;
            microsw::constraints::ConstraintSolver constraintSolver{modelingSolverPolicy()};
            microsw::ActiveExtrusion activeExtrusion;
            microsw::ActiveBoolean activeBoolean;
            presentation.regenerate(sketch);
            microsw::features::FeatureModel featureModel{constraintSolver};
            microsw::sketch::Sketch sketchA;
            (void)sketchA.addRectangle({0, 0}, {4, 4});
            microsw::sketch::Sketch sketchB{{{2, 2, 1}, {1, 0, 0}, {0, 1, 0}}};
            (void)sketchB.addRectangle({0, 0}, {4, 4});
            const auto sketchAId = featureModel.addSketch(std::move(sketchA));
            const auto extrudeAId = featureModel.addExtrude(sketchAId, 4);
            const auto sketchBId = featureModel.addSketch(std::move(sketchB));
            const auto extrudeBId = featureModel.addExtrude(sketchBId, 4);
            (void)featureModel.addBoolean(extrudeAId, extrudeBId,
                microsw::modeling::BooleanOperation::Intersection);
            const auto initialRebuild = featureModel.rebuild();
            if (initialRebuild.status == microsw::features::RebuildStatus::Failed)
                throw std::runtime_error{"Initial parametric feature rebuild failed"};
            microsw::presentation::FeaturePresentation featurePresentation;
            featurePresentation.update(featureModel);
            microsw::PersistenceSession session{std::move(featureModel)};
            microsw::viewer::WorkspaceViewport workspace{featurePresentation.geometry()};

            while (!window.shouldClose())
            {
                window.pollEvents();
                // Whole-frame background before UI composition; the 3D pass clears
                // only its scissored Workspace region.
                openGLContext.clear();
                ui.beginFrame();
                const auto selectedVisual = workspace.selectedEntity();
                const auto selectedSketch = selectedVisual && !activeExtrusion.hasSolid()
                    ? presentation.sketchId(*selectedVisual) : std::nullopt;
                const auto* selected = selectedSketch && sketch.contains(*selectedSketch)
                    ? &sketch.find(*selectedSketch) : nullptr;
                shell.setCurrentFilePath(session.currentFilePath()
                    ? session.currentFilePath()->string() : std::string{});
                shell.drawSketch(workspace.projectionMode(), tools.tool(), sketch, selected,
                    true, &session.model());
                try
                {
                    if (shell.newDocumentRequest())
                    {
                        session.newDocument(); sketch = microsw::sketch::Sketch{};
                        presentation.regenerate(sketch); featurePresentation = {};
                        workspace.setPresentation(featurePresentation.geometry()); workspace.clearSelection();
                        tools.setTool(microsw::SketchTool::Select); shell.reportPersistenceStatus("New document");
                    }
                    if (shell.saveAsDocumentRequest())
                    {
                        const auto result=session.saveAs(shell.documentPathInput());
                        shell.reportPersistenceStatus(result.status==microsw::persistence::SaveStatus::Success
                            ? "Save As succeeded" : "Save As failed: "+result.diagnostic);
                    }
                    if (shell.saveDocumentRequest())
                    {
                        const auto result=session.save();
                        shell.reportPersistenceStatus(result.status==microsw::persistence::SaveStatus::Success
                            ? "Save succeeded" : "Save failed: "+result.diagnostic);
                    }
                    if (shell.openDocumentRequest())
                    {
                        std::string diagnostic; const auto status=session.open(shell.documentPathInput(),&diagnostic);
                        if(status==microsw::persistence::OpenStatus::SuccessRebuilt
                            || status==microsw::persistence::OpenStatus::SuccessRebuildFailed)
                        {
                            featurePresentation={};
                            if(status==microsw::persistence::OpenStatus::SuccessRebuilt)
                                featurePresentation.update(session.model());
                            workspace.setPresentation(featurePresentation.geometry()); workspace.clearSelection();
                            tools.setTool(microsw::SketchTool::Select);
                            shell.reportPersistenceStatus(status==microsw::persistence::OpenStatus::SuccessRebuilt
                                ? "Open succeeded; model rebuilt" : "File loaded; model rebuild failed: "+diagnostic);
                        }
                        else shell.reportPersistenceStatus("Open failed: "+diagnostic);
                    }
                    if (shell.featureDistanceRequest()) session.model().setExtrudeDistance(
                        shell.featureDistanceRequest()->first, shell.featureDistanceRequest()->second);
                    if (shell.featureBooleanRequest()) session.model().setBooleanOperation(
                        shell.featureBooleanRequest()->first, shell.featureBooleanRequest()->second);
                    if (shell.rebuildFeaturesRequest())
                    {
                        const auto result = session.model().rebuild();
                        shell.reportFeatureRebuild(result);
                        if (result.status != microsw::features::RebuildStatus::Failed)
                        {
                            featurePresentation.update(session.model());
                            workspace.setPresentation(featurePresentation.geometry());
                        }
                    }
                }
                catch (const std::exception& error) { shell.reportSketchError(error.what()); }
                if (shell.sketchToolRequest())
                    tools.setTool(*shell.sketchToolRequest());
                if (selectedSketch && shell.sketchGeometryRequest())
                {
                    try
                    {
                        std::visit([&](const auto& geometry)
                        {
                            using Entity = std::decay_t<decltype(geometry)>;
                            if constexpr (std::is_same_v<Entity, microsw::sketch::SketchLine>)
                                sketch.replaceLine(*selectedSketch, geometry.geometry);
                            else if constexpr (std::is_same_v<Entity, microsw::sketch::SketchCircle>)
                                sketch.replaceCircle(*selectedSketch, geometry.geometry);
                            else sketch.replaceArc(*selectedSketch, geometry.geometry);
                        }, *shell.sketchGeometryRequest());
                        presentation.regenerate(sketch);
                    }
                    catch (const std::exception& error) { shell.reportSketchError(error.what()); }
                }
                if (selectedSketch && shell.deleteSketchRequest())
                {
                    sketch.remove(*selectedSketch);
                    presentation.regenerate(sketch);
                    workspace.clearSelection();
                }
                try
                {
                    if (shell.addConstraintRequest()) (void)sketch.addConstraint(*shell.addConstraintRequest());
                    if (shell.removeConstraintRequest()) sketch.removeConstraint(*shell.removeConstraintRequest());
                    if (shell.drivingValueRequest()) sketch.setDrivingValue(
                        shell.drivingValueRequest()->first, shell.drivingValueRequest()->second);
                    if (shell.solveSketchRequest())
                    {
                        const auto result = constraintSolver.solve(sketch);
                        if (result.status == microsw::constraints::SolveStatus::Solved)
                            presentation.regenerate(sketch);
                        else shell.reportSketchError("Constraint solve did not produce a valid solution");
                    }
                }
                catch (const std::exception& error) { shell.reportSketchError(error.what()); }
                if (shell.extrusionRequest())
                {
                    try
                    {
                        const bool firstResult = !activeExtrusion.hasSolid();
                        activeExtrusion.regenerate(sketch, shell.extrusionDistance());
                        if (firstResult)
                            workspace.setPresentation(activeExtrusion.presentation().geometry());
                        shell.reportExtrusionSuccess();
                    }
                    catch (const std::exception& error)
                    {
                        shell.reportExtrusionError(error.what());
                    }
                }
                if (shell.loadBooleanDemoRequest())
                {
                    const microsw::modeling::Profile a{{{0,0,0},{4,0,0},{4,4,0},{0,4,0}},{{0,0,0},{0,0,1}}};
                    const microsw::modeling::Profile b{{{2,2,1},{6,2,1},{6,6,1},{2,6,1}},{{2,2,1},{0,0,1}}};
                    activeBoolean.setOperandA(microsw::modeling::extrude(a,4));
                    activeBoolean.setOperandB(microsw::modeling::extrude(b,4));
                    shell.reportBooleanOperandsReady();
                }
                if (shell.loadBooleanUnsupportedRequest())
                {
                    const microsw::modeling::Profile a{{{0,0,0},{4,0,0},{4,4,0},{0,4,0}},{{0,0,0},{0,0,1}}};
                    const microsw::modeling::Profile b{{{4,0,0},{8,0,0},{8,4,0},{4,4,0}},{{4,0,0},{0,0,1}}};
                    activeBoolean.setOperandA(microsw::modeling::extrude(a,4));
                    activeBoolean.setOperandB(microsw::modeling::extrude(b,4));
                    shell.reportBooleanOperandsReady();
                }
                if (shell.assignOperandARequest() && activeExtrusion.solid())
                    activeBoolean.setOperandA(*activeExtrusion.solid());
                if (shell.assignOperandBRequest() && activeExtrusion.solid())
                    activeBoolean.setOperandB(*activeExtrusion.solid());
                if (shell.booleanRequest())
                {
                    const auto status = activeBoolean.execute(*shell.booleanRequest());
                    shell.reportBooleanStatus(status);
                    if (status == microsw::modeling::BooleanStatus::SUCCESS)
                    {
                        workspace.setPresentation(activeBoolean.presentation().geometry());
                        workspace.clearSelection();
                    }
                }
                workspace.updateNavigation(shell.workspaceRect(), shell.workspaceInput());
                const auto framebuffer = window.framebufferSize();
                workspace.updateHover(shell.workspaceRect(), shell.workspaceInput(),
                    framebuffer.width, framebuffer.height);
                if (tools.tool() == microsw::SketchTool::Select)
                    workspace.updateSelection(shell.workspaceRect(), shell.workspaceInput(),
                        framebuffer.width, framebuffer.height);
                else if (shell.workspaceInput().leftPressed && shell.workspaceInput().focused
                    && shell.workspaceInput().pointerValid && shell.workspaceInput().workspaceHovered
                    && !shell.workspaceInput().blocked)
                {
                    try
                    {
                        const auto world = workspace.pointOnGlobalXY(shell.workspaceRect(),
                            framebuffer.width, framebuffer.height,
                            shell.workspaceInput().x, shell.workspaceInput().y);
                        if (world)
                        {
                            (void)tools.click(sketch.plane().toLocal(*world));
                            presentation.regenerate(sketch);
                        }
                    }
                    catch (const std::exception& error) { shell.reportSketchError(error.what()); }
                }
                workspace.render(shell.workspaceRect(), framebuffer.width, framebuffer.height);
                ui.endFrame();
                window.swapBuffers();
            }
        }

        microsw::Logger::info("Application window closed");
        microsw::Logger::info("Application shutting down");
        return 0;
    }
    catch (const std::exception& error)
    {
        microsw::Logger::error(error.what());
        microsw::Logger::info("Application shutting down");
        return 1;
    }
}
