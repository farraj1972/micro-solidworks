#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"
#include "app/sketch/SketchToolController.h"
#include "presentation/SketchPresentation.h"
#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "viewer/WorkspaceViewport.h"
#include "constraints/ConstraintSolver.h"

#include <exception>
#include <stdexcept>

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
            microsw::constraints::ConstraintSolver constraintSolver;
            presentation.regenerate(sketch);
            microsw::viewer::WorkspaceViewport workspace{presentation.geometry()};

            while (!window.shouldClose())
            {
                window.pollEvents();
                // Whole-frame background before UI composition; the 3D pass clears
                // only its scissored Workspace region.
                openGLContext.clear();
                ui.beginFrame();
                const auto selectedVisual = workspace.selectedEntity();
                const auto selectedSketch = selectedVisual
                    ? presentation.sketchId(*selectedVisual) : std::nullopt;
                const auto* selected = selectedSketch && sketch.contains(*selectedSketch)
                    ? &sketch.find(*selectedSketch) : nullptr;
                shell.drawSketch(workspace.projectionMode(), tools.tool(), sketch, selected);
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
