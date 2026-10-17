#include "presentation/FeaturePresentation.h"
#include <gtest/gtest.h>
namespace { microsw::sketch::Sketch rectangle(double x0,double y0,double x1,double y1){microsw::sketch::Sketch s;(void)s.addRectangle({x0,y0},{x1,y1});return s;} }
TEST(FeaturePresentation,PreservesVisualIdentityAcrossSuccessfulRebuild){
 microsw::features::FeatureModel model;auto s=model.addSketch(rectangle(0,0,4,4));auto e=model.addExtrude(s,4);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);auto id=p.visualId(e);ASSERT_TRUE(id);
 model.setExtrudeDistance(e,6);ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);p.update(model);EXPECT_EQ(p.visualId(e),id);
}
TEST(FeaturePresentation,SuccessfulEmptyRemovesViewportGeometry){
 microsw::features::FeatureModel model;auto a=model.addExtrude(model.addSketch(rectangle(0,0,1,1)),1);auto b=model.addExtrude(model.addSketch(rectangle(3,3,4,4)),1);(void)model.addBoolean(a,b,microsw::modeling::BooleanOperation::Intersection);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::SuccessEmpty);microsw::presentation::FeaturePresentation p;p.update(model);EXPECT_TRUE(p.geometry().empty());
}
TEST(FeaturePresentation,ResolvesVisualProxyBackToLogicalFeature){
 microsw::features::FeatureModel model;auto feature=model.addExtrude(model.addSketch(rectangle(0,0,2,2)),2);
 ASSERT_EQ(model.rebuild().status,microsw::features::RebuildStatus::Success);microsw::presentation::FeaturePresentation p;p.update(model);
 const auto visual=p.visualId(feature);ASSERT_TRUE(visual);EXPECT_EQ(p.featureId(*visual),feature);
 EXPECT_FALSE(p.featureId(microsw::presentation::VisualEntityId{999999}));
}
