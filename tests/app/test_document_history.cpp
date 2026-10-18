#include "app/DocumentHistory.h"
#include "app/PersistenceSession.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace
{
using namespace microsw;

features::FeatureModel modelWithSketch()
{
    features::FeatureModel model;
    sketch::Sketch sketch;
    (void)sketch.addLine({{0, 0}, {1, 0}});
    (void)model.addSketch(std::move(sketch));
    (void)model.rebuild();
    return model;
}

TEST(FeatureCheckpoint, RestoresAuthoritativeIdsCountersAndRequiresDerivedRebuild)
{
    auto model = modelWithSketch();
    const auto checkpoint = model.checkpoint();
    auto& sketch = const_cast<microsw::sketch::Sketch&>(
        std::get<features::SketchFeature>(model.find(features::FeatureId{0}).data()).sketch);
    const auto removed = sketch.addCircle({{2, 2}, 1});
    sketch.remove(removed);
    model.restoreCheckpoint(checkpoint);

    ASSERT_EQ(model.history().size(), 1u);
    EXPECT_EQ(model.nextFeatureId(), 1u);
    EXPECT_FALSE(model.find(features::FeatureId{0}).cache().has_value());
    EXPECT_EQ(model.find(features::FeatureId{0}).state(), features::FeatureState::Dirty);
    EXPECT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    const auto& restored = std::get<features::SketchFeature>(
        model.find(features::FeatureId{0}).data()).sketch;
    EXPECT_EQ(restored.nextEntityId(), 1u);
}

TEST(DocumentHistory, UndoRedoTruncatesFutureAndRestoresFeatureIds)
{
    features::FeatureModel model;
    DocumentHistory history{model};
    const auto first = model.addSketch({}); history.record(model);
    const auto second = model.addSketch({}); history.record(model);
    ASSERT_TRUE(history.undo(model));
    EXPECT_EQ(model.nextFeatureId(), 1u);
    EXPECT_TRUE(history.canRedo());
    const auto replacement = model.addSketch({}); history.record(model);
    EXPECT_EQ(replacement, second);
    EXPECT_EQ(first.value(), 0u);
    EXPECT_FALSE(history.canRedo());
}

TEST(DocumentHistory, RetainsAtMostOneHundredUndoSteps)
{
    auto model = modelWithSketch();
    DocumentHistory history{model};
    for (int i = 0; i < 125; ++i)
    {
        model.editSketch(features::FeatureId{0}, [i](auto& sketch) {
            sketch.replaceLine(sketch.entityIds().front(), {{0, 0}, {static_cast<double>(i + 2), 0}});
        });
        history.record(model);
    }
    EXPECT_EQ(history.undoCount(), DocumentHistory::maximumUndoSteps);
    for (std::size_t i = 0; i < DocumentHistory::maximumUndoSteps; ++i) EXPECT_TRUE(history.undo(model));
    EXPECT_FALSE(history.undo(model));
}

TEST(DocumentHistory, SavedCheckpointSurvivesHistoryEviction)
{
    auto model = modelWithSketch();
    DocumentHistory history{model};
    history.markSaved(model);
    const auto savedCounter = history.savedCheckpoint()->nextFeatureId;
    for (int i = 0; i < 110; ++i)
    {
        model.editSketch(features::FeatureId{0}, [i](auto& sketch) {
            sketch.replaceLine(sketch.entityIds().front(), {{0, 0}, {static_cast<double>(i + 3), 0}});
        });
        history.record(model);
    }
    EXPECT_TRUE(history.isDirty());
    EXPECT_EQ(history.savedCheckpoint()->nextFeatureId, savedCounter);
    EXPECT_EQ(history.undoCount(), DocumentHistory::maximumUndoSteps);
}

TEST(DocumentHistory, SavedBaselineIsIndependentAndDirtyTracksCursor)
{
    auto model = modelWithSketch();
    DocumentHistory history{model};
    EXPECT_FALSE(history.isDirty());
    model.addSketch({}); history.record(model);
    EXPECT_TRUE(history.isDirty());
    history.markSaved(model);
    ASSERT_TRUE(history.savedCheckpoint().has_value());
    EXPECT_FALSE(history.isDirty());
    ASSERT_TRUE(history.undo(model));
    EXPECT_TRUE(history.isDirty());
    ASSERT_TRUE(history.redo(model));
    EXPECT_FALSE(history.isDirty());
}

TEST(PersistenceSessionHistory, FailedRebuildEditRemainsUndoableAndRedoable)
{
    sketch::Sketch sketch;
    const auto circle = sketch.addCircle({{0, 0}, 2});
    (void)sketch.addConstraint(sketch::CircleRadius{{circle, sketch::SubElementKind::CircleRadius}, 3});
    features::FeatureModel model;
    const auto sketchId = model.addSketch(std::move(sketch));
    (void)model.rebuild();
    PersistenceSession session{std::move(model)};
    session.model().editSketch(sketchId, [circle](auto& value) {
        (void)value.addConstraint(sketch::CircleRadius{{circle, sketch::SubElementKind::CircleRadius}, 4});
    });
    EXPECT_EQ(session.model().rebuild().status, features::RebuildStatus::Failed);
    session.recordEdit();
    EXPECT_TRUE(session.undo());
    EXPECT_EQ(session.model().find(sketchId).state(), features::FeatureState::Clean);
    EXPECT_TRUE(session.redo());
    EXPECT_EQ(session.model().find(sketchId).state(), features::FeatureState::Failed);
}

TEST(PersistenceSessionHistory, SaveKeepsHistoryAndTracksSavedCursor)
{
    const auto path = std::filesystem::temp_directory_path() / "microsw-history-save.microsw";
    PersistenceSession session{modelWithSketch()};
    session.model().addSketch({}); session.recordEdit();
    ASSERT_TRUE(session.isDirty());
    ASSERT_EQ(session.saveAs(path).status, persistence::SaveStatus::Success);
    EXPECT_FALSE(session.isDirty());
    EXPECT_TRUE(session.canUndo());
    session.model().addSketch({}); session.recordEdit();
    EXPECT_TRUE(session.isDirty());
    ASSERT_TRUE(session.undo());
    EXPECT_FALSE(session.isDirty());
    ASSERT_TRUE(session.redo());
    EXPECT_TRUE(session.isDirty());
    std::filesystem::remove(path);
}

TEST(PersistenceSessionHistory, FailedOpenPreservesDirtyModelPathAndHistory)
{
    const auto valid = std::filesystem::temp_directory_path() / "microsw-history-valid.microsw";
    const auto invalid = std::filesystem::temp_directory_path() / "microsw-history-invalid.microsw";
    PersistenceSession session{modelWithSketch()};
    ASSERT_EQ(session.saveAs(valid).status, persistence::SaveStatus::Success);
    session.model().addSketch({}); session.recordEdit();
    { std::ofstream output{invalid}; output << "{"; }
    EXPECT_EQ(session.open(invalid), persistence::OpenStatus::ParseError);
    EXPECT_TRUE(session.isDirty());
    EXPECT_TRUE(session.canUndo());
    EXPECT_EQ(session.currentFilePath(), std::optional{valid});
    EXPECT_EQ(session.model().history().size(), 2u);
    std::filesystem::remove(valid); std::filesystem::remove(invalid);
}
}
