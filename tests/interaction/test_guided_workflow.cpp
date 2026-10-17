#include "app/sketch/SketchToolController.h"
#include "features/FeatureModel.h"
#include "interaction/EditorContext.h"
#include "presentation/FeaturePresentation.h"

#include <gtest/gtest.h>

namespace
{
using namespace microsw;

TEST(GuidedWorkflow, CancelledToolStepLeavesAuthoritativeModelUnchanged)
{
    features::FeatureModel model;
    const auto sketchFeature = model.addSketch({});
    auto candidate = std::get<features::SketchFeature>(model.find(sketchFeature).data()).sketch;
    SketchToolController tool{candidate};
    interaction::EditorContext editor;
    editor.activateSketch(sketchFeature);
    editor.activateCommand(interaction::EditorCommand::Line);
    tool.setTool(SketchTool::Line);
    EXPECT_TRUE(tool.click({1, 1}).empty());
    tool.cancel(); editor.escape();
    EXPECT_EQ(candidate.size(), 0u);
    EXPECT_EQ(std::get<features::SketchFeature>(model.find(sketchFeature).data()).sketch.size(), 0u);
}

TEST(GuidedWorkflow, RectangleConstraintsDimensionAndExtrudeCommitThroughFeatureModel)
{
    features::FeatureModel model;
    const auto sketchFeature = model.addSketch({});
    auto candidate = std::get<features::SketchFeature>(model.find(sketchFeature).data()).sketch;
    SketchToolController tool{candidate};
    tool.setTool(SketchTool::Rectangle);
    EXPECT_TRUE(tool.click({0, 0}).empty());
    const auto lines = tool.click({4, 3});
    ASSERT_EQ(lines.size(), 4u);
    (void)candidate.addConstraint(sketch::Coincident{
        {lines[0], sketch::SubElementKind::LineEnd}, {lines[1], sketch::SubElementKind::LineStart}});
    (void)candidate.addConstraint(sketch::Coincident{
        {lines[1], sketch::SubElementKind::LineEnd}, {lines[2], sketch::SubElementKind::LineStart}});
    (void)candidate.addConstraint(sketch::Coincident{
        {lines[2], sketch::SubElementKind::LineEnd}, {lines[3], sketch::SubElementKind::LineStart}});
    (void)candidate.addConstraint(sketch::Coincident{
        {lines[3], sketch::SubElementKind::LineEnd}, {lines[0], sketch::SubElementKind::LineStart}});
    (void)candidate.addConstraint(sketch::Horizontal{{lines[0], sketch::SubElementKind::LineBody}});
    (void)candidate.addConstraint(sketch::Vertical{{lines[1], sketch::SubElementKind::LineBody}});
    const auto dimension = candidate.addConstraint(sketch::HorizontalDistance{
        {lines[0], sketch::SubElementKind::LineStart},
        {lines[0], sketch::SubElementKind::LineEnd}, 4});

    model.editSketch(sketchFeature, [&](auto& authoritative) { authoritative = candidate; });
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    candidate.setDrivingValue(dimension, 6);
    model.editSketch(sketchFeature, [&](auto& authoritative) { authoritative = candidate; });
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    const auto extrude = model.addExtrude(sketchFeature, 5);
    const auto extrusionRebuild = model.rebuild();
    ASSERT_EQ(extrusionRebuild.status, features::RebuildStatus::Success)
        << extrusionRebuild.diagnostic;

    presentation::FeaturePresentation view;
    view.update(model);
    const auto visual = view.visualId(extrude);
    ASSERT_TRUE(visual);
    EXPECT_EQ(view.featureId(*visual), extrude);
    EXPECT_EQ(std::get<sketch::HorizontalDistance>(
        std::get<features::SketchFeature>(model.find(sketchFeature).data()).sketch.findConstraint(dimension).value()).value, 6);
}

TEST(GuidedWorkflow, CoincidentUsesLogicalElementReferencesWithoutIdsInWorkflow)
{
    sketch::Sketch candidate;
    const auto first = candidate.addLine({{0, 0}, {2, 0}});
    const auto second = candidate.addLine({{2.1, 0}, {2.1, 2}});
    interaction::EditorContext editor;
    editor.activateSketch(features::FeatureId{1});
    editor.activateCommand(interaction::EditorCommand::Coincident);
    editor.addOperand({first, sketch::SubElementKind::LineEnd});
    editor.addOperand({second, sketch::SubElementKind::LineStart});
    ASSERT_EQ(editor.operands().size(), 2u);
    const auto constraint = candidate.addConstraint(sketch::Coincident{editor.operands()[0], editor.operands()[1]});
    EXPECT_TRUE(candidate.containsConstraint(constraint));
}
}
