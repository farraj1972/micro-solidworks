#include "ui/FeatureTreeModel.h"

#include <gtest/gtest.h>

namespace
{
microsw::sketch::Sketch rectangle(double x)
{
    microsw::sketch::Sketch sketch;
    (void)sketch.addRectangle({x, 0}, {x + 1, 1});
    return sketch;
}
}

TEST(FeatureTreeModel, UnconsumedSketchRemainsRoot)
{
    microsw::features::FeatureModel model;
    const auto sketch = model.addSketch(rectangle(0));
    const auto tree = microsw::ui::deriveFeatureTree(model);
    ASSERT_EQ(tree.rootName, "Part1");
    ASSERT_EQ(tree.roots.size(), 1u);
    EXPECT_EQ(tree.roots.front().featureId, sketch);
    EXPECT_TRUE(tree.roots.front().children.empty());
}

TEST(FeatureTreeModel, ExtrudeAbsorbsSketchWithoutDuplicate)
{
    microsw::features::FeatureModel model;
    const auto sketch = model.addSketch(rectangle(0));
    const auto extrude = model.addExtrude(sketch, 2);
    model.renameFeature(sketch, "Base Sketch");
    model.renameFeature(extrude, "Base");
    const auto tree = microsw::ui::deriveFeatureTree(model);
    ASSERT_EQ(tree.roots.size(), 1u);
    EXPECT_EQ(tree.roots.front().featureId, extrude);
    EXPECT_EQ(tree.roots.front().displayName, "Base");
    ASSERT_EQ(tree.roots.front().children.size(), 1u);
    EXPECT_EQ(tree.roots.front().children.front().featureId, sketch);
    EXPECT_EQ(tree.roots.front().children.front().displayName, "Base Sketch");
}

TEST(FeatureTreeModel, PreservesRootHistoryAndKeepsBooleanAtRoot)
{
    microsw::features::FeatureModel model;
    const auto sketchA = model.addSketch(rectangle(0));
    const auto extrudeA = model.addExtrude(sketchA, 2);
    const auto sketchB = model.addSketch(rectangle(3));
    const auto extrudeB = model.addExtrude(sketchB, 2);
    const auto boolean = model.addBoolean(extrudeA, extrudeB,
        microsw::modeling::BooleanOperation::Union);
    const auto unused = model.addSketch(rectangle(8));
    const auto tree = microsw::ui::deriveFeatureTree(model);
    ASSERT_EQ(tree.roots.size(), 4u);
    EXPECT_EQ(tree.roots[0].featureId, extrudeA);
    EXPECT_EQ(tree.roots[1].featureId, extrudeB);
    EXPECT_EQ(tree.roots[2].featureId, boolean);
    EXPECT_EQ(tree.roots[3].featureId, unused);
    EXPECT_TRUE(tree.roots[2].children.empty());
}

TEST(FeatureTreeModel, FirstDirectConsumerOwnsSharedSketchPresentation)
{
    microsw::features::FeatureModel model;
    const auto sketch = model.addSketch(rectangle(0));
    const auto first = model.addExtrude(sketch, 2);
    const auto second = model.addExtrude(sketch, 3);
    const auto tree = microsw::ui::deriveFeatureTree(model);
    ASSERT_EQ(tree.roots.size(), 2u);
    EXPECT_EQ(tree.roots[0].featureId, first);
    ASSERT_EQ(tree.roots[0].children.size(), 1u);
    EXPECT_EQ(tree.roots[0].children.front().featureId, sketch);
    EXPECT_EQ(tree.roots[1].featureId, second);
    EXPECT_TRUE(tree.roots[1].children.empty());
}

TEST(FeatureTreeModel, MapsVisibilityStateAndActiveEditing)
{
    microsw::features::FeatureModel model;
    const auto sketch = model.addSketch(rectangle(0));
    const auto extrude = model.addExtrude(sketch, 2);
    microsw::interaction::EditorContext editor;
    editor.setDisplayState(extrude, microsw::interaction::DisplayState::Hidden);
    editor.activateSketch(sketch);
    const auto tree = microsw::ui::deriveFeatureTree(model, &editor);
    ASSERT_EQ(tree.roots.size(), 1u);
    EXPECT_EQ(tree.roots.front().displayState, microsw::interaction::DisplayState::Hidden);
    ASSERT_EQ(tree.roots.front().children.size(), 1u);
    EXPECT_TRUE(tree.roots.front().children.front().activeEditing);
}

TEST(FeatureTreeModel, CutExtrudeAbsorbsItsProfileSketch)
{
    microsw::features::FeatureModel model;
    const auto base=model.addSketch(rectangle(0));
    const auto extrude=model.addExtrude(base,4);
    const auto profile=model.addFaceSketch(rectangle(0.2),extrude,
        microsw::features::ExtrudeCapRole::EndCap);
    const auto cut=model.addCutExtrude(profile,extrude,1,
        microsw::features::CutDirection::Reverse);
    const auto tree=microsw::ui::deriveFeatureTree(model);
    ASSERT_EQ(tree.roots.size(),2u);
    EXPECT_EQ(tree.roots[1].featureId,cut);
    ASSERT_EQ(tree.roots[1].children.size(),1u);
    EXPECT_EQ(tree.roots[1].children.front().featureId,profile);
}

TEST(FeatureTreeModel, BlockedDiagnosticNamesFailedUpstreamFeature)
{
    microsw::features::FeatureModel model;
    const auto source = model.addSketch(rectangle(0));
    const auto extrude = model.addExtrude(source, 2);
    const auto second = model.addExtrude(model.addSketch(rectangle(0.5)), 2);
    const auto dependent = model.addBoolean(extrude, second,
        microsw::modeling::BooleanOperation::Union);
    ASSERT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    model.editSketch(source, [](auto& sketch) { sketch = {}; });
    const auto result = model.rebuild();
    ASSERT_EQ(result.status, microsw::features::RebuildStatus::Failed);
    EXPECT_EQ(model.find(dependent).state(), microsw::features::FeatureState::BlockedByDependency);
    const auto message = microsw::ui::featureStatusDescription(model, dependent);
    EXPECT_NE(message.find("Extrude1"), std::string::npos);
    EXPECT_EQ(message.find("FeatureId"), std::string::npos);
}
