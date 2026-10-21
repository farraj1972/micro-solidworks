#include "app/SketchDimensionEdit.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
using namespace microsw;

struct Fixture
{
    static features::FeatureModel makeModel(bool conflicting)
    {
        features::FeatureModel model;
        sketch::Sketch sketch;
        const auto line = sketch.addLine({{0, 0}, {4, 0}});
        (void)sketch.addConstraint(
            sketch::LineLength{{line, sketch::SubElementKind::LineBody}, 4});
        if (conflicting)
            (void)sketch.addConstraint(
                sketch::LineLength{{line, sketch::SubElementKind::LineBody}, 4});
        (void)model.addSketch(sketch);
        if (model.rebuild().status == features::RebuildStatus::Failed)
            throw std::runtime_error{"Test fixture did not rebuild"};
        return model;
    }

    PersistenceSession session;
    features::FeatureId feature{0};
    sketch::SketchConstraintId dimension{0};

    explicit Fixture(bool conflicting = false) : session{makeModel(conflicting)} {}

    sketch::Sketch solved() const
    {
        return std::get<features::SketchFeatureCache>(
            *session.model().find(feature).cache()).solvedSketch;
    }
};

double value(const PersistenceSession& session, features::FeatureId feature,
    sketch::SketchConstraintId id)
{
    const auto& sketch = std::get<features::SketchFeature>(session.model().find(feature).data()).sketch;
    return std::get<sketch::LineLength>(sketch.findConstraint(id).value()).value;
}

TEST(SketchDimensionEdit, CommitRebuildsSolvedGeometryAndCreatesOneUndoStep)
{
    Fixture fixture;
    const auto before = fixture.solved();
    const auto result = commitSketchDrivingDimension(
        fixture.session, fixture.feature, before, fixture.dimension, 8);
    EXPECT_NE(result.status, features::RebuildStatus::Failed);
    EXPECT_DOUBLE_EQ(value(fixture.session, fixture.feature, fixture.dimension), 8);
    const auto after = fixture.solved();
    const auto line = after.find(after.entityIds().front());
    EXPECT_NEAR(std::get<sketch::SketchLine>(line.geometry()).geometry.length(), 8, 1e-5);
    EXPECT_TRUE(fixture.session.isDirty());
    ASSERT_TRUE(fixture.session.undo());
    EXPECT_DOUBLE_EQ(value(fixture.session, fixture.feature, fixture.dimension), 4);
    ASSERT_TRUE(fixture.session.redo());
    EXPECT_DOUBLE_EQ(value(fixture.session, fixture.feature, fixture.dimension), 8);
}

TEST(SketchDimensionEdit, InvalidOrMissingDimensionDoesNotMutateHistory)
{
    Fixture fixture;
    EXPECT_THROW((void)commitSketchDrivingDimension(fixture.session, fixture.feature,
        fixture.solved(), fixture.dimension, 0), std::invalid_argument);
    EXPECT_THROW((void)commitSketchDrivingDimension(fixture.session, fixture.feature,
        fixture.solved(), sketch::SketchConstraintId{99}, 8), std::invalid_argument);
    EXPECT_DOUBLE_EQ(value(fixture.session, fixture.feature, fixture.dimension), 4);
    EXPECT_FALSE(fixture.session.canUndo());
    EXPECT_FALSE(fixture.session.isDirty());
}

TEST(SketchDimensionEdit, SolverFailurePreservesPriorModel)
{
    Fixture fixture{true};
    EXPECT_THROW((void)commitSketchDrivingDimension(fixture.session, fixture.feature,
        fixture.solved(), fixture.dimension, 8), std::runtime_error);
    EXPECT_DOUBLE_EQ(value(fixture.session, fixture.feature, fixture.dimension), 4);
    EXPECT_FALSE(fixture.session.canUndo());
    EXPECT_FALSE(fixture.session.isDirty());
}

TEST(SketchDimensionEdit, SavedAndReopenedDocumentRetainsCommittedDimension)
{
    Fixture fixture;
    (void)commitSketchDrivingDimension(fixture.session, fixture.feature,
        fixture.solved(), fixture.dimension, 7);
    const auto path = std::filesystem::temp_directory_path() / "ux02-dimension-edit.microsw";
    ASSERT_EQ(fixture.session.saveAs(path).status, persistence::SaveStatus::Success);
    PersistenceSession reopened;
    ASSERT_EQ(reopened.open(path), persistence::OpenStatus::SuccessRebuilt);
    EXPECT_DOUBLE_EQ(value(reopened, fixture.feature, fixture.dimension), 7);
    std::filesystem::remove(path);
}
}
