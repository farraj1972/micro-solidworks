#include "presentation/FeaturePresentation.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
namespace { microsw::sketch::Sketch rectangle(double x0,double y0,double x1,double y1){microsw::sketch::Sketch s;(void)s.addRectangle({x0,y0},{x1,y1});return s;} }
TEST(FeaturePresentation,PreservesVisualIdentityAcrossSuccessfulRebuild){
 microsw::features::FeatureModel model;auto s=model.addSketch(rectangle(0,0,4,4));auto e=model.addExtrude(s,4);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);auto id=p.visualId(e);ASSERT_TRUE(id);
 model.setExtrudeDistance(e,6);ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);p.update(model);EXPECT_EQ(p.visualId(e),id);
}
TEST(FeaturePresentation,SuccessfulEmptyResultDoesNotRemoveOtherVisibleFeatureGeometry){
 microsw::features::FeatureModel model;auto a=model.addExtrude(model.addSketch(rectangle(0,0,1,1)),1);auto b=model.addExtrude(model.addSketch(rectangle(3,3,4,4)),1);(void)model.addBoolean(a,b,microsw::modeling::BooleanOperation::Intersection);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::SuccessEmpty);microsw::presentation::FeaturePresentation p;p.update(model);EXPECT_EQ(p.geometry().size(),2u);EXPECT_TRUE(p.visualId(a));EXPECT_TRUE(p.visualId(b));
}
TEST(FeaturePresentation,ResolvesVisualProxyBackToLogicalFeature){
 microsw::features::FeatureModel model;auto feature=model.addExtrude(model.addSketch(rectangle(0,0,2,2)),2);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);
 const auto visual=p.visualId(feature);ASSERT_TRUE(visual);EXPECT_EQ(p.featureId(*visual),feature);
 EXPECT_FALSE(p.featureId(microsw::presentation::VisualEntityId{999999}));
}
TEST(FeaturePresentation,PresentsEveryBodyUnderOneLogicalFeature){
 microsw::sketch::Sketch sketch;sketch.addCircle({{-3,0},1});sketch.addCircle({{3,0},1});
 microsw::features::FeatureModel model;const auto feature=model.addExtrude(model.addSketch(std::move(sketch)),2);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);
 ASSERT_EQ(p.geometry().size(),2u);for(const auto& entity:p.geometry().entities())EXPECT_EQ(p.featureId(entity.id()),feature);
 const auto first=p.visualId(feature);ASSERT_TRUE(first);model.setExtrudeDistance(feature,3);ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);p.update(model);EXPECT_EQ(p.visualId(feature),first);
}
TEST(FeaturePresentation,PresentsIndependentSolidFeaturesAndAppliesVisibilityPolicy){
 microsw::features::FeatureModel model;const auto a=model.addExtrude(model.addSketch(rectangle(-4,-1,-2,1)),2);const auto b=model.addExtrude(model.addSketch(rectangle(2,-1,4,1)),3);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);
 ASSERT_EQ(p.geometry().size(),2u);ASSERT_TRUE(p.visualId(a));ASSERT_TRUE(p.visualId(b));EXPECT_NE(p.visualId(a),p.visualId(b));
 const auto aVisual=*p.visualId(a);p.update(model,[&](auto id){return id!=b;});EXPECT_EQ(p.geometry().size(),1u);EXPECT_EQ(p.visualId(a),aVisual);EXPECT_FALSE(p.visualId(b));
}
TEST(FeaturePresentation,RemovesDeletedFeatureVisualMappingsImmediately){
 microsw::features::FeatureModel model;const auto sketch=model.addSketch(rectangle(0,0,4,4));const auto solid=model.addExtrude(sketch,4);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);
 const auto visual=p.visualId(solid);ASSERT_TRUE(visual);EXPECT_EQ(p.featureId(*visual),solid);
 (void)model.deleteFeature(solid);p.update(model);
 EXPECT_FALSE(p.visualId(solid));EXPECT_FALSE(p.featureId(*visual));EXPECT_EQ(p.geometry().size(),0u);
}
TEST(FeaturePresentation,PresentsVisibleSketchesAsIndependentReferences){
 microsw::features::FeatureModel model;const auto sketch=model.addSketch(rectangle(0,0,4,4));(void)model.addExtrude(sketch,4);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);
 EXPECT_EQ(p.sketchReferences().size(),4u);
 p.update(model,[&](auto id){return id!=sketch;});EXPECT_TRUE(p.sketchReferences().empty());
}
TEST(FeaturePresentation,PreservesCoplanarHighlightRegionsWhenAggregatingPocketDisplay){
 microsw::features::FeatureModel model;
 const auto base=model.addExtrude(model.addSketch(rectangle(-40,-25,40,25)),8);
 const auto profile=model.addFaceSketch(rectangle(-9,-7,9,7),base,
  microsw::features::ExtrudeCapRole::EndCap);
 const auto cut=model.addCutExtrude(profile,base,3,microsw::features::CutDirection::Reverse);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);
 microsw::presentation::FeaturePresentation presentation;
 presentation.update(model,[&](auto id){return id==cut;});
 const auto& display=presentation.solidDisplay();
 ASSERT_FALSE(display.coplanarNeighbors.empty());
 std::vector<microsw::interaction::TopologyFaceTarget> wallFaces;
 for(const auto& triangle:display.triangles){
  if(std::all_of(triangle.points.begin(),triangle.points.end(),[](const auto& point){
   return std::abs(point.y()+25)<1e-8;
  })&&std::find(wallFaces.begin(),wallFaces.end(),triangle.target)==wallFaces.end())
   wallFaces.push_back(triangle.target);
 }
 ASSERT_GE(wallFaces.size(),2u);
 const auto region=microsw::presentation::faceHighlightRegion(display,wallFaces.front());
 for(const auto face:wallFaces)
  EXPECT_NE(std::find(region.begin(),region.end(),face),region.end());
 EXPECT_EQ(display.triangles.front().target.feature,cut);
}
