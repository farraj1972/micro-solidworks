#include "app/PersistenceSession.h"
#include "app/logging/Logger.h"
#include "app/sketch/SketchToolController.h"
#include "app/window/ApplicationWindow.h"
#include "constraints/ConstraintSolver.h"
#include "core/geometry/GeometricTolerance.h"
#include "interaction/EditorContext.h"
#include "presentation/FeaturePresentation.h"
#include "presentation/SketchPresentation.h"
#include "rendering/OpenGLContext.h"
#include "ui/ApplicationShell.h"
#include "ui/ImGuiLayer.h"
#include "viewer/WorkspaceViewport.h"
#include <exception>
#include <optional>
#include <stdexcept>
#include <type_traits>

namespace {
microsw::constraints::SolverPolicy solverPolicy(){ microsw::constraints::SolverPolicy p; p.residualSatisfactionTolerance=microsw::geometry::defaultGeometricTolerance; p.stepTolerance=microsw::geometry::defaultGeometricTolerance*0.01; p.costReductionTolerance=1e-16; p.initialStateRegularization=1e-12; p.maxIterations=200; return p; }
bool hasSolid(const microsw::features::FeatureModel& m){ if(!m.resultFeatureId().isValid()) return false; const auto& r=m.find(m.resultFeatureId()); return r.cache()&&std::holds_alternative<microsw::features::SolidFeatureCache>(*r.cache()); }
}

int main(){
 microsw::Logger::info("Application starting");
 try{
  microsw::ApplicationWindow window{1280,720,"Micro SolidWorks"}; microsw::OpenGLContext gl{window}; microsw::ImGuiLayer ui{window,gl}; microsw::ApplicationShell shell{window};
  microsw::constraints::ConstraintSolver solver{solverPolicy()}; microsw::PersistenceSession session{microsw::features::FeatureModel{solver}}; microsw::interaction::EditorContext editor;
  microsw::sketch::Sketch workingSketch; microsw::SketchToolController tools{workingSketch}; microsw::presentation::SketchPresentation sketchView; microsw::presentation::FeaturePresentation featureView; microsw::presentation::GeometryPresentation hiddenView;
  featureView.update(session.model()); sketchView.regenerate(workingSketch); microsw::viewer::WorkspaceViewport workspace{featureView.geometry()};
  const auto refreshFeatures=[&]{featureView.update(session.model());workspace.setPresentation(featureView.geometry());};
  const auto refreshSketch=[&]{sketchView.regenerate(workingSketch);workspace.setPresentation(sketchView.geometry());};
  const auto commitSketch=[&]{ microsw::features::RebuildResult result; if(const auto id=editor.activeSketchFeatureId()){session.model().editSketch(*id,[&](auto& s){s=workingSketch;});result=session.model().rebuild();shell.reportFeatureRebuild(result);refreshSketch();} return result; };
  const auto enterSketch=[&](std::optional<microsw::features::FeatureId> requested){ microsw::features::FeatureId id; if(requested&&session.model().find(*requested).kind()==microsw::features::FeatureKind::Sketch)id=*requested;else id=session.model().addSketch({}); workingSketch=std::get<microsw::features::SketchFeature>(session.model().find(id).data()).sketch; editor.activateSketch(id);shell.setSelectedFeature(id);tools.setTool(microsw::SketchTool::Select);refreshSketch(); };
  while(!window.shouldClose()){
   window.pollEvents();gl.clear();ui.beginFrame();
   const auto selectedVisual=workspace.selectedEntity(); const auto selectedSketchId=editor.activeSketchFeatureId()&&selectedVisual?sketchView.sketchId(*selectedVisual):std::nullopt; const auto* selected=selectedSketchId&&workingSketch.contains(*selectedSketchId)?&workingSketch.find(*selectedSketchId):nullptr;
   shell.setCurrentFilePath(session.currentFilePath()?session.currentFilePath()->string():std::string{}); shell.drawSketch(workspace.projectionMode(),tools.tool(),workingSketch,selected,hasSolid(session.model()),&session.model(),&editor);
   try{
    if(shell.newDocumentRequest()){session.newDocument();editor.resetSession();workingSketch={};sketchView={};featureView={};shell.setSelectedFeature(std::nullopt);featureView.update(session.model());workspace.setPresentation(featureView.geometry());tools.setTool(microsw::SketchTool::Select);shell.reportPersistenceStatus("New document — choose Sketch to begin");}
    if(shell.saveAsDocumentRequest()){const auto r=session.saveAs(shell.documentPathInput());shell.reportPersistenceStatus(r.status==microsw::persistence::SaveStatus::Success?"Save As succeeded":"Save As failed: "+r.diagnostic);}
    if(shell.saveDocumentRequest()){const auto r=session.save();shell.reportPersistenceStatus(r.status==microsw::persistence::SaveStatus::Success?"Save succeeded":"Save failed: "+r.diagnostic);}
    if(shell.openDocumentRequest()){std::string d;const auto s=session.open(shell.documentPathInput(),&d);if(s==microsw::persistence::OpenStatus::SuccessRebuilt||s==microsw::persistence::OpenStatus::SuccessRebuildFailed){editor.resetSession();shell.setSelectedFeature(std::nullopt);featureView={};if(s==microsw::persistence::OpenStatus::SuccessRebuilt)refreshFeatures();else workspace.setPresentation(hiddenView);shell.reportPersistenceStatus(s==microsw::persistence::OpenStatus::SuccessRebuilt?"Open succeeded":"File opened; rebuild needs attention: "+d);}else shell.reportPersistenceStatus("Open failed: "+d);}
    if(shell.enterSketchRequest())enterSketch(shell.selectedFeature());
    if(shell.exitSketchRequest()&&editor.activeSketchFeatureId()){(void)commitSketch();editor.exitSketch();refreshFeatures();}
    if(shell.editorCommandRequest()){using EC=microsw::interaction::EditorCommand;editor.activateCommand(*shell.editorCommandRequest());switch(*shell.editorCommandRequest()){case EC::Line:tools.setTool(microsw::SketchTool::Line);break;case EC::Rectangle:tools.setTool(microsw::SketchTool::Rectangle);break;case EC::Circle:tools.setTool(microsw::SketchTool::Circle);break;default:tools.setTool(microsw::SketchTool::Select);break;}if(*shell.editorCommandRequest()==EC::Extrude)editor.beginPropertyEdit({microsw::interaction::PropertyEditContext::Kind::ExtrudeDistance,shell.extrusionDistance()});}
    if(shell.cancelPropertyRequest()){editor.cancelStep();tools.cancel();}
    if(shell.workspaceInput().escapePressed){editor.escape();tools.cancel();if(editor.activeCommand()==microsw::interaction::EditorCommand::Select)tools.setTool(microsw::SketchTool::Select);}
    if(selectedSketchId&&shell.sketchGeometryRequest()){std::visit([&](const auto& g){using E=std::decay_t<decltype(g)>;if constexpr(std::is_same_v<E,microsw::sketch::SketchLine>)workingSketch.replaceLine(*selectedSketchId,g.geometry);else if constexpr(std::is_same_v<E,microsw::sketch::SketchCircle>)workingSketch.replaceCircle(*selectedSketchId,g.geometry);else workingSketch.replaceArc(*selectedSketchId,g.geometry);},*shell.sketchGeometryRequest());(void)commitSketch();}
    if(selectedSketchId&&shell.deleteSketchRequest()){workingSketch.remove(*selectedSketchId);(void)commitSketch();workspace.clearSelection();}
    if(shell.addConstraintRequest()){(void)workingSketch.addConstraint(*shell.addConstraintRequest());editor.clearOperands();(void)commitSketch();}
    if(shell.removeConstraintRequest()){workingSketch.removeConstraint(*shell.removeConstraintRequest());(void)commitSketch();}
    if(shell.drivingValueRequest()){workingSketch.setDrivingValue(shell.drivingValueRequest()->first,shell.drivingValueRequest()->second);(void)commitSketch();}
    if(shell.solveSketchRequest())(void)commitSketch();
    if(shell.featureDistanceRequest()){session.model().setExtrudeDistance(shell.featureDistanceRequest()->first,shell.featureDistanceRequest()->second);const auto r=session.model().rebuild();shell.reportFeatureRebuild(r);if(r.status!=microsw::features::RebuildStatus::Failed)refreshFeatures();}
    if(shell.rebuildFeaturesRequest()){const auto r=session.model().rebuild();shell.reportFeatureRebuild(r);if(r.status!=microsw::features::RebuildStatus::Failed){if(editor.activeSketchFeatureId())refreshSketch();else refreshFeatures();}}
    if(shell.extrusionRequest()){const auto source=editor.activeSketchFeatureId().value_or(shell.selectedFeature().value_or(microsw::features::FeatureId{}));if(!source.isValid()||session.model().find(source).kind()!=microsw::features::FeatureKind::Sketch)throw std::invalid_argument{"Select a valid Sketch before Extrude"};(void)commitSketch();const auto extrude=session.model().addExtrude(source,shell.extrusionDistance());const auto r=session.model().rebuild();shell.reportFeatureRebuild(r);if(r.status==microsw::features::RebuildStatus::Failed)throw std::runtime_error{r.diagnostic};editor.exitSketch();editor.select(microsw::interaction::LogicalTarget{extrude});shell.setSelectedFeature(extrude);refreshFeatures();if(const auto visual=featureView.visualId(extrude))workspace.selectEntity(visual);shell.reportExtrusionSuccess();}
    if(shell.visibilityToggleRequest()){const auto f=*shell.visibilityToggleRequest();const bool hide=editor.displayState(f)!=microsw::interaction::DisplayState::Hidden;editor.setDisplayState(f,hide?microsw::interaction::DisplayState::Hidden:microsw::interaction::DisplayState::Visible);if(f==session.model().resultFeatureId())workspace.setPresentation(hide?hiddenView:featureView.geometry());}
   }catch(const std::exception& e){shell.reportSketchError(e.what());}
   workspace.updateNavigation(shell.workspaceRect(),shell.workspaceInput());const auto fb=window.framebufferSize();workspace.updateHover(shell.workspaceRect(),shell.workspaceInput(),fb.width,fb.height);
   if(const auto hovered=workspace.hoveredEntity()){if(editor.activeSketchFeatureId()){if(const auto id=sketchView.sketchId(*hovered))editor.setHover(microsw::interaction::LogicalTarget{*id});}else if(const auto id=featureView.featureId(*hovered))editor.setHover(microsw::interaction::LogicalTarget{*id});}else editor.setHover(std::nullopt);
   if(tools.tool()==microsw::SketchTool::Select){workspace.updateSelection(shell.workspaceRect(),shell.workspaceInput(),fb.width,fb.height);if(shell.workspaceInput().leftPressed){if(editor.activeSketchFeatureId()){if(const auto visual=workspace.selectedEntity())if(const auto id=sketchView.sketchId(*visual)){editor.select(microsw::interaction::LogicalTarget{*id});if(editor.activeCommand()==microsw::interaction::EditorCommand::Coincident)if(const auto world=workspace.pointOnGlobalXY(shell.workspaceRect(),fb.width,fb.height,shell.workspaceInput().x,shell.workspaceInput().y))if(const auto element=microsw::interaction::nearestElement(workingSketch,*id,workingSketch.plane().toLocal(*world),0.25))editor.addOperand(*element);}}else if(const auto visual=workspace.selectedEntity())if(const auto id=featureView.featureId(*visual)){editor.select(microsw::interaction::LogicalTarget{*id});shell.setSelectedFeature(id);}}}
   else if(shell.workspaceInput().leftPressed&&shell.workspaceInput().focused&&shell.workspaceInput().pointerValid&&shell.workspaceInput().workspaceHovered&&!shell.workspaceInput().blocked){try{if(const auto world=workspace.pointOnGlobalXY(shell.workspaceRect(),fb.width,fb.height,shell.workspaceInput().x,shell.workspaceInput().y)){auto point=workingSketch.plane().toLocal(*world);const auto snap=microsw::interaction::resolveSnap(workingSketch,point,0.2,1.0,shell.gridSnapEnabled());editor.setSnapCandidate(snap);if(snap)point=snap->point;std::optional<microsw::interaction::InferenceCandidate> inference;if(const auto anchor=tools.pendingPoint()){inference=microsw::interaction::resolveInference(*anchor,point,0.15,snap);if(inference)point=inference->point;}editor.setInferenceCandidate(inference);const auto created=tools.click(point);if(!created.empty()){if(inference&&tools.tool()==microsw::SketchTool::Line){const microsw::sketch::SketchElementRef body{created.front(),microsw::sketch::SubElementKind::LineBody};if(inference->kind==microsw::interaction::InferenceKind::Horizontal)(void)workingSketch.addConstraint(microsw::sketch::Horizontal{body});if(inference->kind==microsw::interaction::InferenceKind::Vertical)(void)workingSketch.addConstraint(microsw::sketch::Vertical{body});}if(tools.tool()==microsw::SketchTool::Rectangle){using SEK=microsw::sketch::SubElementKind;(void)workingSketch.addConstraint(microsw::sketch::Coincident{{created[0],SEK::LineEnd},{created[1],SEK::LineStart}});(void)workingSketch.addConstraint(microsw::sketch::Coincident{{created[1],SEK::LineEnd},{created[2],SEK::LineStart}});(void)workingSketch.addConstraint(microsw::sketch::Coincident{{created[2],SEK::LineEnd},{created[3],SEK::LineStart}});(void)workingSketch.addConstraint(microsw::sketch::Coincident{{created[3],SEK::LineEnd},{created[0],SEK::LineStart}});(void)workingSketch.addConstraint(microsw::sketch::Horizontal{{created[0],SEK::LineBody}});(void)workingSketch.addConstraint(microsw::sketch::Vertical{{created[1],SEK::LineBody}});(void)workingSketch.addConstraint(microsw::sketch::Horizontal{{created[2],SEK::LineBody}});(void)workingSketch.addConstraint(microsw::sketch::Vertical{{created[3],SEK::LineBody}});}(void)commitSketch();}}}catch(const std::exception& e){shell.reportSketchError(e.what());}}
   if(!editor.activeSketchFeatureId()&&shell.selectedFeature())if(const auto visual=featureView.visualId(*shell.selectedFeature()))if(workspace.selectedEntity()!=visual)workspace.selectEntity(visual);
   workspace.render(shell.workspaceRect(),fb.width,fb.height);ui.endFrame();window.swapBuffers();
  }
  microsw::Logger::info("Application window closed");microsw::Logger::info("Application shutting down");return 0;
 }catch(const std::exception& e){microsw::Logger::error(e.what());microsw::Logger::info("Application shutting down");return 1;}
}
