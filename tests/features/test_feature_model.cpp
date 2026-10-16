#include "features/FeatureModel.h"

#include <gtest/gtest.h>

namespace
{
microsw::sketch::Sketch rectangle(double x0, double y0, double x1, double y1)
{
    microsw::sketch::Sketch result;
    (void)result.addRectangle({x0, y0}, {x1, y1});
    return result;
}
}

TEST(FeatureModel, OwnsStableAppendOnlyTypedHistory)
{
    microsw::features::FeatureModel model;
    const auto sketchA = model.addSketch(rectangle(0, 0, 4, 4));
    const auto extrudeA = model.addExtrude(sketchA, 4);
    const auto sketchB = model.addSketch(rectangle(2, 2, 6, 6));
    const auto extrudeB = model.addExtrude(sketchB, 4);
    const auto boolean = model.addBoolean(extrudeA, extrudeB, microsw::modeling::BooleanOperation::Union);
    EXPECT_EQ(model.history().size(), 5U);
    EXPECT_EQ(model.find(sketchA).displayName(), "Sketch1");
    EXPECT_EQ(model.find(sketchB).displayName(), "Sketch2");
    EXPECT_EQ(model.find(boolean).dependencies(), (std::vector{extrudeA, extrudeB}));
    EXPECT_EQ(model.resultFeatureId(), boolean);
    for (const auto& feature : model.history()) EXPECT_EQ(feature.state(), microsw::features::FeatureState::Dirty);
}

TEST(FeatureModel, RejectsInvalidAndWrongKindDependencies)
{
    microsw::features::FeatureModel model;
    const auto sketch = model.addSketch(rectangle(0, 0, 4, 4));
    const auto extrude = model.addExtrude(sketch, 4);
    EXPECT_THROW(model.addExtrude({}, 4), std::out_of_range);
    EXPECT_THROW(model.addExtrude(extrude, 4), std::invalid_argument);
    EXPECT_THROW(model.addBoolean(sketch, extrude, microsw::modeling::BooleanOperation::Union), std::invalid_argument);
    EXPECT_THROW(model.setResultFeatureId(sketch), std::invalid_argument);
}

TEST(FeatureModel, RebuildsBranchesAndPropagatesDirtyState)
{
    microsw::features::FeatureModel model;
    const auto sketchA = model.addSketch(rectangle(0, 0, 4, 4));
    const auto extrudeA = model.addExtrude(sketchA, 4);
    const auto sketchB = model.addSketch(rectangle(2, 2, 6, 6));
    const auto extrudeB = model.addExtrude(sketchB, 4);
    const auto boolean = model.addBoolean(extrudeA, extrudeB, microsw::modeling::BooleanOperation::Intersection);
    EXPECT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    model.setExtrudeDistance(extrudeA, 5);
    EXPECT_EQ(model.find(extrudeA).state(), microsw::features::FeatureState::Dirty);
    EXPECT_EQ(model.find(boolean).state(), microsw::features::FeatureState::Dirty);
    EXPECT_EQ(model.find(extrudeB).state(), microsw::features::FeatureState::Clean);
    EXPECT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    EXPECT_EQ(model.find(extrudeA).id(), extrudeA);
}

TEST(FeatureModel, FailedClosurePreservesAllLastSuccessfulCachesAndRecovers)
{
    microsw::features::FeatureModel model;
    const auto sketchA = model.addSketch(rectangle(0, 0, 4, 4));
    const auto extrudeA = model.addExtrude(sketchA, 4);
    const auto sketchB = model.addSketch(rectangle(2, 2, 6, 6));
    const auto extrudeB = model.addExtrude(sketchB, 4);
    const auto boolean = model.addBoolean(extrudeA, extrudeB, microsw::modeling::BooleanOperation::Union);
    ASSERT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    const auto oldA = &*model.find(extrudeA).cache();
    const auto oldBoolean = &*model.find(boolean).cache();
    model.editSketch(sketchA, [](auto& sketch) { sketch.remove(sketch.entityIds().front()); });
    const auto failed = model.rebuild();
    EXPECT_EQ(failed.status, microsw::features::RebuildStatus::Failed);
    EXPECT_EQ(model.find(sketchA).state(), microsw::features::FeatureState::Dirty);
    EXPECT_EQ(model.find(extrudeA).state(), microsw::features::FeatureState::Failed);
    EXPECT_EQ(model.find(boolean).state(), microsw::features::FeatureState::BlockedByDependency);
    EXPECT_EQ(&*model.find(extrudeA).cache(), oldA);
    EXPECT_EQ(&*model.find(boolean).cache(), oldBoolean);
    model.editSketch(sketchA, [](auto& sketch) { sketch = rectangle(0, 0, 5, 4); });
    EXPECT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    EXPECT_EQ(model.find(boolean).state(), microsw::features::FeatureState::Clean);
}

TEST(FeatureModel, SuccessfulEmptyReplacesPreviousCurrentResult)
{
    microsw::features::FeatureModel model;
    const auto a = model.addExtrude(model.addSketch(rectangle(0, 0, 1, 1)), 1);
    const auto b = model.addExtrude(model.addSketch(rectangle(3, 3, 4, 4)), 1);
    const auto result = model.addBoolean(a, b, microsw::modeling::BooleanOperation::Intersection);
    EXPECT_EQ(model.rebuild().status, microsw::features::RebuildStatus::SuccessEmpty);
    ASSERT_TRUE(model.find(result).cache());
    EXPECT_TRUE(std::holds_alternative<microsw::features::EmptyFeatureCache>(*model.find(result).cache()));
    EXPECT_EQ(model.find(result).state(), microsw::features::FeatureState::Clean);
}

TEST(FeatureModel, SolvesSketchCopyWithoutMutatingAuthoritativeGeometry)
{
    microsw::features::FeatureModel model;
    microsw::sketch::Sketch sketch;
    const auto line = sketch.addLine({{0, 0}, {1, 0}});
    (void)sketch.addConstraint(microsw::sketch::LineLength{
        {line, microsw::sketch::SubElementKind::LineBody}, 2});
    const auto feature = model.addSketch(sketch);
    ASSERT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    const auto& authoritative = std::get<microsw::sketch::SketchLine>(
        std::get<microsw::features::SketchFeature>(model.find(feature).data()).sketch.find(line).geometry()).geometry;
    const auto& solved = std::get<microsw::sketch::SketchLine>(
        std::get<microsw::features::SketchFeatureCache>(*model.find(feature).cache()).solvedSketch.find(line).geometry()).geometry;
    EXPECT_NEAR(authoritative.length(), 1.0, 1e-12);
    EXPECT_NEAR(solved.length(), 2.0, 1e-7);
}

TEST(FeatureModel, FailureRollsBackSuccessfulCandidateOnIndependentAffectedBranch)
{
    microsw::features::FeatureModel model;
    const auto sketchA = model.addSketch(rectangle(0, 0, 4, 4));
    const auto extrudeA = model.addExtrude(sketchA, 4);
    const auto sketchB = model.addSketch(rectangle(2, 2, 6, 6));
    const auto extrudeB = model.addExtrude(sketchB, 4);
    (void)model.addBoolean(extrudeA, extrudeB, microsw::modeling::BooleanOperation::Union);
    ASSERT_EQ(model.rebuild().status, microsw::features::RebuildStatus::Success);
    const auto* previousB = &*model.find(extrudeB).cache();
    model.editSketch(sketchA, [](auto& sketch) { sketch.remove(sketch.entityIds().front()); });
    model.setExtrudeDistance(extrudeB, 6);
    const auto result = model.rebuild();
    EXPECT_EQ(result.status, microsw::features::RebuildStatus::Failed);
    EXPECT_EQ(&*model.find(extrudeB).cache(), previousB);
    EXPECT_EQ(model.find(extrudeB).state(), microsw::features::FeatureState::Dirty);
}
