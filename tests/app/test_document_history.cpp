#include "app/DocumentHistory.h"
#include "app/PersistenceSession.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cmath>

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

TEST(PersistenceSessionHistory, CutCreateEditAndUpstreamChangeUndoRedoAsCadTransactions)
{
    features::FeatureModel model;
    sketch::Sketch base;(void)base.addRectangle({0,0},{10,10});
    const auto target=model.addExtrude(model.addSketch(std::move(base)),10);
    sketch::Sketch profile;(void)profile.addCircle({{5,5},2});
    const auto source=model.addFaceSketch(std::move(profile),target,features::ExtrudeCapRole::EndCap);
    ASSERT_EQ(model.rebuild().status,features::RebuildStatus::Success);
    PersistenceSession session{std::move(model)};
    const auto cut=session.model().addCutExtrude(source,target,4,features::CutDirection::Reverse);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);session.recordEdit();
    ASSERT_TRUE(session.undo());EXPECT_THROW((void)session.model().find(cut),std::out_of_range);
    ASSERT_TRUE(session.redo());ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);
    session.model().setCutExtrude(cut,6,features::CutDirection::Reverse);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);session.recordEdit();
    ASSERT_TRUE(session.undo());
    EXPECT_DOUBLE_EQ(std::get<features::CutExtrudeFeature>(session.model().find(cut).data()).distance,4);
    session.model().setExtrudeDistance(target,20);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);session.recordEdit();
    ASSERT_TRUE(session.undo());
    EXPECT_DOUBLE_EQ(std::get<features::ExtrudeFeature>(session.model().find(target).data()).distance,10);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);
}

TEST(PersistenceSessionHistory, HoledReferencePlateSurvivesEditsHistoryAndVersionTwoRoundtrip)
{
    PersistenceSession session;
    sketch::Sketch base;
    const auto sides=base.addRectangle({-40,-25},{40,25});
    const auto hole=base.addCircle({{-22,0},4});
    const auto baseId=session.model().addSketch(std::move(base));
    const auto extrude=session.model().addExtrude(baseId,8);
    sketch::Sketch pocket;
    (void)pocket.addRectangle({7,-7},{25,7});
    const auto source=session.model().addFaceSketch(std::move(pocket),extrude,
        features::ExtrudeCapRole::EndCap);
    const auto cut=session.model().addCutExtrude(source,extrude,2,
        features::CutDirection::Reverse);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);
    session.recordEdit();

    const auto check=[&](double halfWidth,double depth){
        const auto& record=session.model().find(cut);
        EXPECT_EQ(record.state(),features::FeatureState::Clean);
        EXPECT_EQ(record.dependencies(),(std::vector{source,extrude}));
        const auto& support=std::get<features::SketchFeature>(session.model().find(source).data()).support;
        const auto* semantic=std::get_if<features::SemanticFaceSupport>(&support);
        ASSERT_NE(semantic,nullptr);
        EXPECT_EQ(semantic->sourceFeatureId,extrude);
        EXPECT_EQ(semantic->capRole,features::ExtrudeCapRole::EndCap);
        const auto& cache=std::get<features::SolidFeatureCache>(*record.cache());
        ASSERT_EQ(cache.solids.size(),1u);
        const auto& solid=cache.solids.front();
        EXPECT_TRUE(solid.isValid());
        EXPECT_NEAR(features::resolveSemanticFacePlane(cache,
            features::ExtrudeCapRole::PocketFloor).origin().z(),8-depth,1e-9);
        double xmin=1e99,xmax=-1e99,ymin=1e99,ymax=-1e99,zmin=1e99,zmax=-1e99;
        bool holeCylinder=false;
        for(const auto& vertex:solid.vertices()){
            const auto& p=vertex.point();
            xmin=std::min(xmin,p.x());xmax=std::max(xmax,p.x());
            ymin=std::min(ymin,p.y());ymax=std::max(ymax,p.y());
            zmin=std::min(zmin,p.z());zmax=std::max(zmax,p.z());
        }
        for(const auto& face:solid.faces())
            if(const auto* cylinder=std::get_if<geometry::Cylinder>(&face.support());
                cylinder && std::abs(cylinder->origin().x()+22)<1e-9
                && std::abs(cylinder->origin().y())<1e-9
                && std::abs(cylinder->radius()-4)<1e-9)holeCylinder=true;
        EXPECT_TRUE(holeCylinder);
        EXPECT_NEAR(xmin,-halfWidth,1e-9);EXPECT_NEAR(xmax,halfWidth,1e-9);
        EXPECT_NEAR(ymin,-25,1e-9);EXPECT_NEAR(ymax,25,1e-9);
        EXPECT_NEAR(zmin,0,1e-9);EXPECT_NEAR(zmax,8,1e-9);
        const auto wireArea=[&](topology::WireId id){
            double area=0;
            for(const auto& use:solid.find(id).uses()){
                const auto& edge=solid.find(use.edge);
                const auto sign=use.orientation==topology::EdgeOrientation::Forward?1.0:-1.0;
                if(const auto* arc=std::get_if<geometry::CircularArc3>(&edge.geometry())){
                    const auto a=arc->pointAt(0),b=arc->pointAt(1),c=arc->center();
                    area+=sign*(c.x()*(b.y()-a.y())-c.y()*(b.x()-a.x())
                        +arc->radius()*arc->radius()*arc->sweepAngle()*arc->normal().z())/2;
                }else{
                    const auto& a=solid.find(edge.start()).point();
                    const auto& b=solid.find(edge.end()).point();
                    area+=sign*(a.x()*b.y()-b.x()*a.y())/2;
                }
            }
            return area;
        };
        double cadVolume=0;
        for(const auto& faceUse:solid.rootShell().uses()){
            const auto& face=solid.find(faceUse.face);
            const auto* plane=std::get_if<geometry::Plane>(&face.support());
            if(!plane || std::abs(plane->normal().z())<0.9)continue;
            double area=wireArea(face.outerWire());
            for(const auto inner:face.innerWires())area+=wireArea(inner);
            const auto sign=faceUse.orientation==topology::FaceOrientation::Forward?1.0:-1.0;
            cadVolume+=sign*plane->origin().z()*area;
        }
        const auto expected=2*halfWidth*50*8-std::acos(-1.0)*4*4*8-18*14*depth;
        EXPECT_NEAR(cadVolume,expected,1e-6);
    };
    check(40,2);
    session.model().editSketch(baseId,[&](auto& sketch){
        sketch.replaceMany({
            {sides[0],sketch::SketchLine{{{-45,-25},{45,-25}}}},
            {sides[1],sketch::SketchLine{{{45,-25},{45,25}}}},
            {sides[2],sketch::SketchLine{{{45,25},{-45,25}}}},
            {sides[3],sketch::SketchLine{{{-45,25},{-45,-25}}}}
        });
    });
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);
    session.recordEdit();check(45,2);
    ASSERT_TRUE(session.undo());check(40,2);
    ASSERT_TRUE(session.redo());check(45,2);
    session.model().setCutExtrude(cut,3,features::CutDirection::Reverse);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);
    session.recordEdit();check(45,3);
    ASSERT_TRUE(session.undo());check(45,2);
    ASSERT_TRUE(session.redo());check(45,3);
    const auto path=std::filesystem::temp_directory_path()/"microsw-r2-reference-plate.microsw";
    ASSERT_EQ(session.saveAs(path).status,persistence::SaveStatus::Success);
    session.newDocument();
    std::string diagnostic;
    ASSERT_EQ(session.open(path,&diagnostic),persistence::OpenStatus::SuccessRebuilt)
        << diagnostic;
    EXPECT_EQ(session.model().history().size(),4u);
    EXPECT_EQ(session.model().resultFeatureId(),cut);
    const auto& restoredSketch=std::get<features::SketchFeature>(session.model().find(baseId).data()).sketch;
    EXPECT_TRUE(restoredSketch.contains(hole));
    for(const auto side:sides)EXPECT_TRUE(restoredSketch.contains(side));
    check(45,3);
    ASSERT_EQ(session.model().rebuild().status,features::RebuildStatus::Success);
    check(45,3);
    std::filesystem::remove(path);
}

TEST(DocumentHistory, DeletionUndoRedoRestoresExactClosureAndRetiredCounter)
{
    features::FeatureModel model;
    sketch::Sketch base; (void)base.addRectangle({0, 0}, {10, 10});
    const auto baseId = model.addExtrude(model.addSketch(std::move(base)), 10);
    sketch::Sketch profile; (void)profile.addRectangle({2, 2}, {8, 8});
    const auto supported = model.addFaceSketch(std::move(profile), baseId,
        features::ExtrudeCapRole::EndCap);
    const auto cut = model.addCutExtrude(supported, baseId, 2,
        features::CutDirection::Reverse);
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    DocumentHistory history{model};

    EXPECT_EQ(model.deleteFeature(supported), (std::vector{supported, cut}));
    history.record(model);
    EXPECT_TRUE(history.undo(model));
    EXPECT_EQ(model.nextFeatureId(), 4u);
    EXPECT_EQ(model.find(cut).dependencies(), (std::vector{supported, baseId}));
    EXPECT_EQ(model.resultFeatureId(), cut);
    EXPECT_TRUE(history.redo(model));
    EXPECT_EQ(model.nextFeatureId(), 4u);
    EXPECT_THROW((void)model.find(supported), std::out_of_range);
    EXPECT_EQ(model.resultFeatureId(), baseId);
}

TEST(PersistenceSessionHistory, RenameIsOneUndoStepAndSurvivesSaveOpen)
{
    const auto path = std::filesystem::temp_directory_path() / "microsw-c5-rename.microsw";
    PersistenceSession session{modelWithSketch()};
    const auto id = session.model().history().front().id();
    session.model().renameFeature(id, "Base Sketch");
    session.recordEdit();
    ASSERT_TRUE(session.undo());
    EXPECT_EQ(session.model().find(id).displayName(), "Sketch1");
    ASSERT_TRUE(session.redo());
    EXPECT_EQ(session.model().find(id).displayName(), "Base Sketch");
    ASSERT_EQ(session.saveAs(path).status, persistence::SaveStatus::Success);
    session.newDocument();
    ASSERT_EQ(session.open(path), persistence::OpenStatus::SuccessRebuilt);
    EXPECT_EQ(session.model().find(id).displayName(), "Base Sketch");
    std::filesystem::remove(path);
}
}
