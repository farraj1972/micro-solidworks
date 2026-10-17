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
    {std::ofstream out(path);out<<R"({"formatVersion":2,"featureModel":{}})";}
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

TEST(Persistence, GeneralizedExtrudeRoundtripsWithFormatVersionOne)
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
    EXPECT_NE(json.find("\"formatVersion\": 1"), std::string::npos);
    auto opened = persistence::openFeatureModel(path);
    ASSERT_EQ(opened.status, persistence::OpenStatus::SuccessRebuilt)
        << opened.diagnostic;
    const auto& cache = std::get<features::SolidFeatureCache>(
        *opened.model.find(extrude).cache());
    EXPECT_EQ(cache.solids.size(), 2u);
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
