#include "interaction/EditorContext.h"

#include <gtest/gtest.h>

namespace
{
using namespace microsw;
using namespace microsw::interaction;

TEST(EditorContext, SeparatesActiveEditSelectionAndTransientCommandState)
{
    EditorContext context;
    const features::FeatureId sketchFeature{3};
    context.activateSketch(sketchFeature);
    context.select(LogicalTarget{sketch::SketchEntityId{7}});
    context.activateCommand(EditorCommand::Coincident);
    context.addOperand({sketch::SketchEntityId{7}, sketch::SubElementKind::LineEnd});
    EXPECT_EQ(context.activeSketchFeatureId(), sketchFeature);
    ASSERT_TRUE(context.selection());
    EXPECT_TRUE(std::holds_alternative<sketch::SketchEntityId>(*context.selection()));
    EXPECT_EQ(context.toolStep(), ToolStep::PickSecond);
    context.escape();
    EXPECT_EQ(context.activeCommand(), EditorCommand::Coincident);
    EXPECT_TRUE(context.operands().empty());
    context.escape();
    EXPECT_EQ(context.activeCommand(), EditorCommand::Select);
    EXPECT_EQ(context.activeSketchFeatureId(), sketchFeature);
}

TEST(EditorContext, TopologySelectionKnowsOwnerAndClearsOnRebuild)
{
    interaction::EditorContext editor;
    const interaction::TopologyFaceTarget face{features::FeatureId{5},{1},topology::FaceId{3}};
    editor.select(interaction::LogicalTarget{face});
    ASSERT_TRUE(editor.selection());
    EXPECT_EQ(editor.activeFeatureId(),features::FeatureId{5});
    editor.clearTopologySelectionOnRebuild();
    EXPECT_FALSE(editor.selection());
    editor.select(interaction::LogicalTarget{features::FeatureId{5}});
    editor.clearTopologySelectionOnRebuild();
    EXPECT_TRUE(editor.selection());
}

TEST(EditorContext, CommandAvailabilityIsContextSensitive)
{
    EditorContext context;
    EXPECT_FALSE(context.commandAvailable(EditorCommand::Line, false));
    EXPECT_FALSE(context.commandAvailable(EditorCommand::Extrude, true));
    context.activateSketch(features::FeatureId{1});
    EXPECT_TRUE(context.commandAvailable(EditorCommand::Rectangle, false));
    EXPECT_TRUE(context.commandAvailable(EditorCommand::Extrude, true));
}

TEST(EditorContext, SessionResetClearsOnlyTransientEditorState)
{
    EditorContext context;
    const features::FeatureId feature{2};
    context.activateSketch(feature);
    context.setDisplayState(feature, DisplayState::Hidden);
    context.beginPropertyEdit({PropertyEditContext::Kind::ExtrudeDistance, 4});
    context.resetSession();
    EXPECT_FALSE(context.activeSketchFeatureId());
    EXPECT_FALSE(context.selection());
    EXPECT_FALSE(context.propertyEdit());
    EXPECT_EQ(context.displayState(feature), DisplayState::Visible);
}

TEST(EditorContext, PreviewLifecycleIsTransientAndCancelledAtomically)
{
    EditorContext context;
    context.activateSketch(features::FeatureId{1});
    context.activateCommand(EditorCommand::Rectangle);
    context.beginPreview({120, 80});
    context.updatePreview({310, 240});
    ASSERT_TRUE(context.previewAnchor());
    ASSERT_TRUE(context.previewCursor());
    EXPECT_DOUBLE_EQ(context.previewAnchor()->x, 120);
    EXPECT_DOUBLE_EQ(context.previewCursor()->y, 240);
    context.cancelStep();
    EXPECT_FALSE(context.previewAnchor());
    EXPECT_FALSE(context.previewCursor());
    EXPECT_EQ(context.toolStep(), ToolStep::PickFirst);
}

TEST(Snapping, UsesDeterministicEndpointMidpointGridPriority)
{
    sketch::Sketch model;
    const auto line = model.addLine({{0, 0}, {10, 0}});
    auto candidate = resolveSnap(model, {0.1, 0.1}, 0.5, 1, true);
    ASSERT_TRUE(candidate); EXPECT_EQ(candidate->kind, SnapKind::Endpoint);
    ASSERT_TRUE(candidate->element); EXPECT_EQ(candidate->element->entity, line);
    candidate = resolveSnap(model, {5.1, 0.1}, 0.5, 1, true);
    ASSERT_TRUE(candidate); EXPECT_EQ(candidate->kind, SnapKind::Midpoint);
    candidate = resolveSnap(model, {3.1, 2.9}, 0.5, 1, true);
    ASSERT_TRUE(candidate); EXPECT_EQ(candidate->kind, SnapKind::Grid);
}

TEST(Inference, ResolvesCoincidentBeforeAxisInference)
{
    const SnapCandidate snap{SnapKind::Endpoint, {4, 4},
        sketch::SketchElementRef{sketch::SketchEntityId{1}, sketch::SubElementKind::LineStart}};
    auto inference = resolveInference({0, 0}, {4, 0.01}, 0.1, snap);
    ASSERT_TRUE(inference); EXPECT_EQ(inference->kind, InferenceKind::Coincident);
    inference = resolveInference({0, 0}, {4, 0.01}, 0.1, std::nullopt);
    ASSERT_TRUE(inference); EXPECT_EQ(inference->kind, InferenceKind::Horizontal);
    inference = resolveInference({0, 0}, {0.01, 4}, 0.1, std::nullopt);
    ASSERT_TRUE(inference); EXPECT_EQ(inference->kind, InferenceKind::Vertical);
}
}
