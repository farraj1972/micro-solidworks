#include "core/topology/CuboidBuilder.h"
#include "presentation/SolidDisplay.h"
#include "viewer/SolidPicker.h"
#include <gtest/gtest.h>
namespace {
using namespace microsw;
TEST(SolidDisplay, CuboidDerivesFacesEdgesNormalsAndLogicalMappings)
{
 const auto solid=topology::makeCuboid(2,4,6);const features::FeatureId feature{7};
 const auto display=presentation::deriveSolidDisplay(solid,feature,{2});
 EXPECT_EQ(display.triangles.size(),12u);EXPECT_EQ(display.edges.size(),12u);
 for(const auto&t:display.triangles){EXPECT_EQ(t.target.feature,feature);EXPECT_EQ(t.target.body.index,2u);EXPECT_NEAR(t.normal.length(),1.0,1e-12);}
 for(const auto&e:display.edges){EXPECT_EQ(e.target.feature,feature);EXPECT_EQ(e.target.body.index,2u);EXPECT_EQ(e.points.size(),2u);}
}
TEST(SolidPicker, CenterSelectsNearestVisibleFaceAndReturnsLogicalTarget)
{
 const auto display=presentation::deriveSolidDisplay(topology::makeCuboid(2,2,2),features::FeatureId{3},{0});
 viewer::PickingContext context;context.camera=viewer::OrbitCamera{{},10,0,0};context.width=800;context.height=600;context.mouseX=400;context.mouseY=300;
 const auto hit=viewer::pickSolid(display,context);ASSERT_TRUE(hit);ASSERT_TRUE(std::holds_alternative<interaction::TopologyFaceTarget>(hit->target));
 EXPECT_EQ(std::get<interaction::TopologyFaceTarget>(hit->target).feature,features::FeatureId{3});
}
TEST(SolidPicker, EmptyDisplayHasNoHit)
{
 viewer::PickingContext context;context.camera=viewer::OrbitCamera{{},10,0,0};context.width=800;context.height=600;context.mouseX=400;context.mouseY=300;
 EXPECT_FALSE(viewer::pickSolid({},context));
}
}
