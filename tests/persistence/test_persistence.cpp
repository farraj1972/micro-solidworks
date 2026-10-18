#include "persistence/Persistence.h"
#include "app/PersistenceSession.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace
{
using namespace microsw;

std::filesystem::path temporaryFile(const char* name)
{
    return std::filesystem::temp_directory_path() / name;
}

features::FeatureModel sampleModel(bool rebuild = true)
{
    features::FeatureModel model;
    sketch::Sketch a;
    const auto lines = a.addRectangle({0, 0}, {4, 4});
    (void)a.addConstraint(sketch::Horizontal{{lines[0], sketch::SubElementKind::LineBody}});
    (void)a.addConstraint(sketch::LineLength{{lines[0], sketch::SubElementKind::LineBody}, 4});
    sketch::Sketch b{{{2, 2, 1}, {1, 0, 0}, {0, 1, 0}}};
    (void)b.addRectangle({0, 0}, {4, 4});
    const auto sa = model.addSketch(std::move(a));
    const auto ea = model.addExtrude(sa, 4);
    const auto sb = model.addSketch(std::move(b));
    const auto eb = model.addExtrude(sb, 4);
    (void)model.addBoolean(ea, eb, modeling::BooleanOperation::Intersection);
    if (rebuild) EXPECT_NE(model.rebuild().status, features::RebuildStatus::Failed);
    return model;
}

TEST(Persistence, RoundtripPreservesAuthoritativeModelAndRebuilds)
{
    const auto path = temporaryFile("micro-solidworks-roundtrip.microsw");
    auto model = sampleModel();
    ASSERT_EQ(persistence::saveFeatureModel(model, path).status, persistence::SaveStatus::Success);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    ASSERT_EQ(opened.model.history().size(), model.history().size());
    EXPECT_EQ(opened.model.nextFeatureId(), model.nextFeatureId());
    EXPECT_EQ(opened.model.resultFeatureId(), model.resultFeatureId());
    for (std::size_t i = 0; i < model.history().size(); ++i) {
        EXPECT_EQ(opened.model.history()[i].id(), model.history()[i].id());
        EXPECT_EQ(opened.model.history()[i].kind(), model.history()[i].kind());
        EXPECT_EQ(opened.model.history()[i].displayName(), model.history()[i].displayName());
    }
    const auto& original = std::get<features::SketchFeature>(model.history()[0].data()).sketch;
    const auto& restored = std::get<features::SketchFeature>(opened.model.history()[0].data()).sketch;
    EXPECT_EQ(restored.entityIds(), original.entityIds());
    EXPECT_EQ(restored.constraintIds(), original.constraintIds());
    EXPECT_EQ(restored.nextEntityId(), original.nextEntityId());
    EXPECT_EQ(restored.nextConstraintId(), original.nextConstraintId());
    std::filesystem::remove(path);
}

TEST(Persistence, PreservesSparseSketchIdCounters)
{
    sketch::Sketch sketch;
    const auto removed = sketch.addLine({{0,0},{1,0}});
    (void)sketch.addLine({{0,0},{0,1}});
    sketch.remove(removed);
    features::FeatureModel model;
    (void)model.addSketch(std::move(sketch));
    const auto path = temporaryFile("micro-solidworks-id-counter.microsw");
    ASSERT_EQ(persistence::saveFeatureModel(model,path).status,persistence::SaveStatus::Success);
    auto opened=persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status,persistence::OpenStatus::SuccessRebuilt);
    auto& restored=const_cast<sketch::Sketch&>(std::get<features::SketchFeature>(opened.model.history()[0].data()).sketch);
    EXPECT_EQ(restored.addLine({{2,0},{3,0}}).value(),2U);
    std::filesystem::remove(path);
}

TEST(Persistence, SaveDoesNotRequireCleanModel)
{
    auto model=sampleModel(false);
    const auto path=temporaryFile("micro-solidworks-dirty.microsw");
    EXPECT_EQ(persistence::saveFeatureModel(model,path).status,persistence::SaveStatus::Success);
    std::filesystem::remove(path);
}

TEST(Persistence, DistinguishesInvalidJsonVersionAndInvalidModel)
{
    const auto path=temporaryFile("micro-solidworks-invalid.microsw");
    {std::ofstream out(path);out<<"{";}
    EXPECT_EQ(persistence::openFeatureModel(path).status,persistence::OpenStatus::ParseError);
    {std::ofstream out(path);out<<R"({"formatVersion":3,"featureModel":{}})";}
    EXPECT_EQ(persistence::openFeatureModel(path).status,persistence::OpenStatus::UnsupportedVersion);
    {std::ofstream out(path);out<<R"({"formatVersion":1,"featureModel":{"nextFeatureId":0,"resultFeatureId":null,"features":[{"id":0,"kind":"mystery","displayName":"X"}]}})";}
    EXPECT_EQ(persistence::openFeatureModel(path).status,persistence::OpenStatus::InvalidModel);
    std::filesystem::remove(path);
}

TEST(Persistence, FailedRebuildIsSuccessfulStructuralOpen)
{
    features::FeatureModel model;
    sketch::Sketch sketch;
    const auto circle=sketch.addCircle({{0,0},2});
    (void)sketch.addConstraint(sketch::CircleRadius{{circle,sketch::SubElementKind::CircleRadius},3});
    (void)sketch.addConstraint(sketch::CircleRadius{{circle,sketch::SubElementKind::CircleRadius},4});
    const auto sid=model.addSketch(std::move(sketch));
    (void)model.addExtrude(sid,4);
    const auto path=temporaryFile("micro-solidworks-rebuild-failed.microsw");
    ASSERT_EQ(persistence::saveFeatureModel(model,path).status,persistence::SaveStatus::Success);
    EXPECT_EQ(persistence::openFeatureModel(path).status,persistence::OpenStatus::SuccessRebuildFailed);
    std::filesystem::remove(path);
}

TEST(Persistence, GeneralizedExtrudeRoundtripsWithFormatVersionTwo)
{
    features::FeatureModel model;
    sketch::Sketch sketch;
    (void)sketch.addCircle({{-3, 0}, 1});
    (void)sketch.addCircle({{3, 0}, 1});
    const auto extrude = model.addExtrude(model.addSketch(std::move(sketch)), 4);
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    const auto path = temporaryFile("micro-solidworks-general-extrude.microsw");

    ASSERT_EQ(persistence::saveFeatureModel(model, path).status,
        persistence::SaveStatus::Success);
    std::ifstream input{path};
    const std::string json{std::istreambuf_iterator<char>{input}, {}};
    input.close();
    EXPECT_NE(json.find("\"formatVersion\": 2"), std::string::npos);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt)
        << opened.diagnostic;
    const auto& cache = std::get<features::SolidFeatureCache>(
        *opened.model.find(extrude).cache());
    EXPECT_EQ(cache.solids.size(), 2u);
    std::filesystem::remove(path);
}

TEST(Persistence, SemanticFaceSupportRoundtripsInVersionTwo)
{
    features::FeatureModel model;
    const auto base = model.addExtrude(model.addSketch([] {
        sketch::Sketch s; (void)s.addRectangle({0, 0}, {4, 4}); return s; }()), 5);
    sketch::Sketch local; (void)local.addCircle({{1, 1}, 0.5});
    const auto faceSketch = model.addFaceSketch(std::move(local), base,
        features::ExtrudeCapRole::EndCap);
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    const auto path = temporaryFile("micro-solidworks-v2-semantic-face.microsw");
    ASSERT_EQ(persistence::saveFeatureModel(model, path).status, persistence::SaveStatus::Success);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    const auto& restored = std::get<features::SketchFeature>(opened.model.find(faceSketch).data());
    const auto* support = std::get_if<features::SemanticFaceSupport>(&restored.support);
    ASSERT_NE(support, nullptr);
    EXPECT_EQ(support->sourceFeatureId, base);
    EXPECT_EQ(support->capRole, features::ExtrudeCapRole::EndCap);
    std::filesystem::remove(path);
}

TEST(Persistence, OpensVersionOneAsExplicitPlaneAndSavesVersionTwo)
{
    const auto path = temporaryFile("micro-solidworks-v1-migration.microsw");
    { std::ofstream out(path); out << R"({"formatVersion":1,"featureModel":{"nextFeatureId":1,"resultFeatureId":null,"features":[{"id":0,"displayName":"Sketch1","kind":"sketch","sketch":{"plane":{"origin":{"x":0,"y":0,"z":0},"xAxis":{"x":1,"y":0,"z":0},"yAxis":{"x":0,"y":1,"z":0}},"nextEntityId":0,"nextConstraintId":0,"entities":[],"constraints":[]}}]}})"; }
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    const auto& sketchFeature = std::get<features::SketchFeature>(opened.model.find(features::FeatureId{0}).data());
    EXPECT_TRUE(std::holds_alternative<features::ExplicitPlaneSupport>(sketchFeature.support));
    ASSERT_EQ(persistence::saveFeatureModel(opened.model, path).status, persistence::SaveStatus::Success);
    std::ifstream input{path}; const std::string json{std::istreambuf_iterator<char>{input}, {}}; input.close();
    EXPECT_NE(json.find("\"formatVersion\": 2"), std::string::npos);
    std::filesystem::remove(path);
}

TEST(Persistence, VersionTwoPreservesCutExtrudeContract)
{
    features::FeatureModel model;
    sketch::Sketch base; (void)base.addRectangle({0, 0}, {4, 4});
    const auto target = model.addExtrude(model.addSketch(std::move(base)), 4);
    sketch::Sketch profile; (void)profile.addCircle({{2, 2}, 1});
    const auto source = model.addFaceSketch(std::move(profile), target,
        features::ExtrudeCapRole::EndCap);
    const auto cut = model.addCutExtrude(source, target, 2, features::CutDirection::Reverse);
    const auto path = temporaryFile("micro-solidworks-v2-cut-contract.microsw");
    ASSERT_EQ(persistence::saveFeatureModel(model, path).status, persistence::SaveStatus::Success);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    const auto& restored = std::get<features::CutExtrudeFeature>(opened.model.find(cut).data());
    EXPECT_EQ(restored.sourceSketchFeatureId, source);
    EXPECT_EQ(restored.targetResultFeatureId, target);
    EXPECT_DOUBLE_EQ(restored.distance, 2);
    EXPECT_EQ(restored.direction, features::CutDirection::Reverse);
    const auto& cache=std::get<features::SolidFeatureCache>(*opened.model.find(cut).cache());
    ASSERT_EQ(cache.solids.size(),1u);
    EXPECT_TRUE(cache.solids.front().isValid());
    std::filesystem::remove(path);
}

TEST(Persistence, DeletedFeaturesStayAbsentAndFeatureIdCounterRemainsAdvanced)
{
    features::FeatureModel model;
    const auto kept = model.addSketch({});
    const auto removed = model.addSketch({});
    (void)model.deleteFeature(removed);
    const auto path = temporaryFile("micro-solidworks-v2-deleted-features.microsw");
    ASSERT_EQ(persistence::saveFeatureModel(model, path).status, persistence::SaveStatus::Success);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    ASSERT_EQ(opened.model.history().size(), 1u);
    EXPECT_EQ(opened.model.history().front().id(), kept);
    EXPECT_EQ(opened.model.nextFeatureId(), 2u);
    EXPECT_THROW((void)opened.model.find(removed), std::out_of_range);
    EXPECT_EQ(opened.model.addSketch({}).value(), 2u);
    std::filesystem::remove(path);
}

TEST(Persistence, PocketFloorSemanticSupportRoundtripsInVersionTwo)
{
    features::FeatureModel model;
    sketch::Sketch base; (void)base.addRectangle({0, 0}, {10, 10});
    const auto target = model.addExtrude(model.addSketch(std::move(base)), 10);
    sketch::Sketch profile; (void)profile.addCircle({{5, 5}, 2});
    const auto profileId = model.addFaceSketch(std::move(profile), target,
        features::ExtrudeCapRole::EndCap);
    const auto cut = model.addCutExtrude(profileId, target, 4,
        features::CutDirection::Reverse);
    sketch::Sketch floorGeometry; (void)floorGeometry.addCircle({{5, 5}, 1});
    const auto floor = model.addFaceSketch(std::move(floorGeometry), cut,
        features::ExtrudeCapRole::PocketFloor);
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    const auto path = temporaryFile("micro-solidworks-v2-pocket-floor.microsw");

    ASSERT_EQ(persistence::saveFeatureModel(model, path).status, persistence::SaveStatus::Success);
    std::ifstream input{path}; const std::string json{std::istreambuf_iterator<char>{input}, {}};
    input.close();
    EXPECT_NE(json.find("\"pocketFloor\""), std::string::npos);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    const auto* support = std::get_if<features::SemanticFaceSupport>(
        &std::get<features::SketchFeature>(opened.model.find(floor).data()).support);
    ASSERT_NE(support, nullptr);
    EXPECT_EQ(support->sourceFeatureId, cut);
    EXPECT_EQ(support->capRole, features::ExtrudeCapRole::PocketFloor);
    EXPECT_EQ(std::get<features::SketchFeature>(opened.model.find(floor).data()).sketch.size(), 1u);
    std::filesystem::remove(path);
}

TEST(Persistence, RejectsUnknownVersionTwoSemanticFaceRole)
{
    const auto path = temporaryFile("micro-solidworks-v2-unknown-face-role.microsw");
    std::ofstream output{path};
    output << R"({"formatVersion":2,"featureModel":{"nextFeatureId":2,"resultFeatureId":null,"features":[{"id":0,"displayName":"Extrude1","kind":"extrude","sourceSketchFeatureId":1,"distance":2},{"id":1,"displayName":"Sketch1","kind":"sketch","sketch":{"plane":{"origin":{"x":0,"y":0,"z":0},"xAxis":{"x":1,"y":0,"z":0},"yAxis":{"x":0,"y":1,"z":0}},"nextEntityId":0,"nextConstraintId":0,"entities":[],"constraints":[]},"support":{"kind":"semanticFace","sourceFeatureId":0,"capRole":"nearestFace"}}]}})";
    output.close();
    EXPECT_EQ(persistence::openFeatureModel(path).status, persistence::OpenStatus::InvalidModel);
    std::filesystem::remove(path);
}

TEST(Persistence, InheritedCapSemanticRolesRoundtripInVersionTwo)
{
    features::FeatureModel model;
    sketch::Sketch base; (void)base.addRectangle({0, 0}, {10, 10});
    const auto target = model.addExtrude(model.addSketch(std::move(base)), 10);
    sketch::Sketch profile; (void)profile.addCircle({{5, 5}, 2});
    const auto profileId = model.addFaceSketch(std::move(profile), target,
        features::ExtrudeCapRole::EndCap);
    const auto cut = model.addCutExtrude(profileId, target, 4,
        features::CutDirection::Reverse);
    const auto start = model.addFaceSketch({}, cut,
        features::ExtrudeCapRole::InheritedStartCap);
    const auto end = model.addFaceSketch({}, cut,
        features::ExtrudeCapRole::InheritedEndCap);
    ASSERT_EQ(model.rebuild().status, features::RebuildStatus::Success);
    const auto path = temporaryFile("micro-solidworks-v2-inherited-caps.microsw");
    ASSERT_EQ(persistence::saveFeatureModel(model, path).status, persistence::SaveStatus::Success);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt) << opened.diagnostic;
    const auto role = [&](features::FeatureId id) {
        return std::get<features::SemanticFaceSupport>(
            std::get<features::SketchFeature>(opened.model.find(id).data()).support).capRole;
    };
    EXPECT_EQ(role(start), features::ExtrudeCapRole::InheritedStartCap);
    EXPECT_EQ(role(end), features::ExtrudeCapRole::InheritedEndCap);
    std::filesystem::remove(path);
}

TEST(Persistence, InvalidPathDoesNotMutateModel)
{
    auto model=sampleModel(); const auto count=model.history().size();
    const auto result=persistence::saveFeatureModel(model,{});
    EXPECT_EQ(result.status,persistence::SaveStatus::InvalidPath);
    EXPECT_EQ(model.history().size(),count);
}

TEST(PersistenceSession, SaveAsSaveOpenAndNewOwnCurrentPathTransactionally)
{
    const auto path=temporaryFile("micro-solidworks-session.microsw");
    PersistenceSession session{sampleModel()};
    ASSERT_EQ(session.save().status,persistence::SaveStatus::InvalidPath);
    ASSERT_EQ(session.saveAs(path).status,persistence::SaveStatus::Success);
    ASSERT_EQ(session.currentFilePath(),std::optional{path});
    session.model().setExtrudeDistance(features::FeatureId{1},5);
    ASSERT_EQ(session.save().status,persistence::SaveStatus::Success);
    session.newDocument();
    EXPECT_TRUE(session.model().history().empty()); EXPECT_FALSE(session.currentFilePath());
    EXPECT_EQ(session.open(path),persistence::OpenStatus::SuccessRebuilt);
    EXPECT_EQ(session.currentFilePath(),std::optional{path});
    EXPECT_EQ(std::get<features::ExtrudeFeature>(session.model().find(features::FeatureId{1}).data()).distance,5);
    std::filesystem::remove(path);
}

TEST(PersistenceSession, InvalidOpenPreservesModelAndPath)
{
    const auto valid=temporaryFile("micro-solidworks-session-valid.microsw");
    const auto invalid=temporaryFile("micro-solidworks-session-invalid.microsw");
    PersistenceSession session{sampleModel()}; ASSERT_EQ(session.saveAs(valid).status,persistence::SaveStatus::Success);
    const auto count=session.model().history().size(); {std::ofstream out(invalid);out<<"{";}
    EXPECT_EQ(session.open(invalid),persistence::OpenStatus::ParseError);
    EXPECT_EQ(session.model().history().size(),count); EXPECT_EQ(session.currentFilePath(),std::optional{valid});
    std::filesystem::remove(valid);std::filesystem::remove(invalid);
}
}
