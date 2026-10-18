#include "core/topology/CuboidBuilder.h"
#include "presentation/SolidDisplay.h"
#include "viewer/SolidPicker.h"
#include "features/GeneralExtrusion.h"
#include "features/PlanarRegion.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <numbers>
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
TEST(SolidDisplay, HoledCapLeavesHoleEmptyAndCylinderIsTessellated)
{
 sketch::Sketch sketch;(void)sketch.addRectangle({-5,-5},{5,5});(void)sketch.addCircle({{0,0},2});
 const auto extrusion=features::extrudeRegions(features::extractPlanarRegions(sketch),sketch.plane(),3);ASSERT_TRUE(extrusion.succeeded());
 const auto display=presentation::deriveSolidDisplay(extrusion.solids.front(),features::FeatureId{9},{0});
 EXPECT_FALSE(display.triangles.empty());
 EXPECT_FALSE(display.edges.empty());
 EXPECT_GT(std::count_if(display.edges.begin(),display.edges.end(),[](const auto&e){return e.points.size()>2;}),0);
 std::size_t bottomTriangles{},topTriangles{};double bottomArea{},topArea{};
 for(const auto&t:display.triangles){const auto z=(t.points[0].z()+t.points[1].z()+t.points[2].z())/3;if(std::abs(z)<1e-8||std::abs(z-3)<1e-8){const auto x=(t.points[0].x()+t.points[1].x()+t.points[2].x())/3;const auto y=(t.points[0].y()+t.points[1].y()+t.points[2].y())/3;EXPECT_GE(std::hypot(x,y),1.99);}}
 for(const auto&t:display.triangles){const auto z=(t.points[0].z()+t.points[1].z()+t.points[2].z())/3;const auto ab=t.points[1]-t.points[0],ac=t.points[2]-t.points[0];const auto area=math::cross(ab,ac).length()/2;if(std::abs(z)<1e-8){++bottomTriangles;bottomArea+=area;}if(std::abs(z-3)<1e-8){++topTriangles;topArea+=area;}}
 const auto expectedCapArea=100.0-24.0*4.0*std::sin(std::numbers::pi/24.0);
 EXPECT_GT(bottomTriangles,0u);EXPECT_GT(topTriangles,0u);
 EXPECT_NEAR(bottomArea,expectedCapArea,1e-8);EXPECT_NEAR(topArea,expectedCapArea,1e-8);
 EXPECT_GT(std::count_if(display.triangles.begin(),display.triangles.end(),[](const auto&t){return std::abs(t.normal.z())<0.9;}),0);
 for(const auto&t:display.triangles)if(std::abs(t.normal.z())<0.9){const auto x=(t.points[0].x()+t.points[1].x()+t.points[2].x())/3;const auto y=(t.points[0].y()+t.points[1].y()+t.points[2].y())/3;EXPECT_GT(t.normal.x()*x+t.normal.y()*y,0);}
}
TEST(SolidDisplay, DistinguishesBodiesFromOneFeature)
{
 const auto a=presentation::deriveSolidDisplay(topology::makeCuboid(1,1,1),features::FeatureId{4},{0});
 const auto b=presentation::deriveSolidDisplay(topology::makeCuboid(1,1,1),features::FeatureId{4},{1});
 ASSERT_FALSE(a.triangles.empty());ASSERT_FALSE(b.triangles.empty());ASSERT_FALSE(a.edges.empty());ASSERT_FALSE(b.edges.empty());EXPECT_NE(a.triangles.front().target.body,b.triangles.front().target.body);
}
}
