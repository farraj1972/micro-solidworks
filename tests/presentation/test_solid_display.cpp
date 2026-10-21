#include "core/topology/CuboidBuilder.h"
#include "presentation/SolidDisplay.h"
#include "viewer/SolidPicker.h"
#include "features/GeneralExtrusion.h"
#include "features/PlanarRegion.h"
#include "features/FeatureModel.h"
#include "features/AxialSubtraction.h"
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
TEST(SolidDisplay, BlindPocketDoesNotExposeCoplanarOuterFaceSeams)
{
 features::FeatureModel model;sketch::Sketch base;(void)base.addRectangle({0,0},{10,10});
 const auto target=model.addExtrude(model.addSketch(std::move(base)),10);
 sketch::Sketch profile;(void)profile.addCircle({{5,5},2});
 const auto sketchId=model.addFaceSketch(std::move(profile),target,features::ExtrudeCapRole::EndCap);
 const auto cut=model.addCutExtrude(sketchId,target,4,features::CutDirection::Reverse);
 ASSERT_EQ(model.rebuild().status,features::RebuildStatus::Success);
 const auto&solid=std::get<features::SolidFeatureCache>(*model.find(cut).cache()).solids.front();
 const auto display=presentation::deriveSolidDisplay(solid,cut,{0});
 const auto outerSeam=[](const presentation::PresentedEdge&e){
  if(e.points.size()!=2)return false;const auto&a=e.points[0];const auto&b=e.points[1];
  if(std::abs(a.z()-6)>1e-8||std::abs(b.z()-6)>1e-8)return false;
  return (std::abs(a.x())<1e-8&&std::abs(b.x())<1e-8)||(std::abs(a.x()-10)<1e-8&&std::abs(b.x()-10)<1e-8)
   ||(std::abs(a.y())<1e-8&&std::abs(b.y())<1e-8)||(std::abs(a.y()-10)<1e-8&&std::abs(b.y()-10)<1e-8);
 };
 EXPECT_EQ(std::count_if(display.edges.begin(),display.edges.end(),outerSeam),0);
 EXPECT_GT(std::count_if(display.edges.begin(),display.edges.end(),[](const auto&e){return e.points.size()>2;}),0);
}
TEST(SolidDisplay, RectangularPocketSplitsOuterWallIntoCoplanarFaceTargetsWithoutVisibleSeam)
{
 features::FeatureModel model;
 sketch::Sketch base;(void)base.addRectangle({0,0},{10,10});
 const auto target=model.addExtrude(model.addSketch(std::move(base)),10);
 sketch::Sketch pocket;(void)pocket.addRectangle({3,3},{7,7});
 const auto source=model.addFaceSketch(std::move(pocket),target,features::ExtrudeCapRole::EndCap);
 const auto cut=model.addCutExtrude(source,target,4,features::CutDirection::Reverse);
 ASSERT_EQ(model.rebuild().status,features::RebuildStatus::Success);
 const auto& solid=std::get<features::SolidFeatureCache>(*model.find(cut).cache()).solids.front();
 ASSERT_TRUE(solid.isValid());
 const auto display=presentation::deriveSolidDisplay(solid,cut,{0});
 std::vector<topology::FaceId> outerWallFaces;
 for(const auto& triangle:display.triangles){
  if(std::all_of(triangle.points.begin(),triangle.points.end(),[](const auto&point){return std::abs(point.y())<1e-8;}))
   if(std::find(outerWallFaces.begin(),outerWallFaces.end(),triangle.target.face)==outerWallFaces.end())
    outerWallFaces.push_back(triangle.target.face);
 }
 EXPECT_GE(outerWallFaces.size(),2u);
 for(const auto face:outerWallFaces){
  const interaction::TopologyFaceTarget targetFace{cut,{0},face};
  EXPECT_GT(std::count_if(display.edges.begin(),display.edges.end(),[&](const auto& edge){
   return std::find(edge.incidentFaces.begin(),edge.incidentFaces.end(),targetFace)
    !=edge.incidentFaces.end();
  }),0);
 }
 const auto artificialFloorSeam=std::count_if(display.edges.begin(),display.edges.end(),[](const auto& edge){
  if(edge.points.size()!=2)return false;
  const auto& a=edge.points[0];const auto& b=edge.points[1];
  return std::abs(a.y())<1e-8&&std::abs(b.y())<1e-8
   &&std::abs(a.z()-6)<1e-8&&std::abs(b.z()-6)<1e-8;
 });
 EXPECT_EQ(artificialFloorSeam,0);
}
TEST(SolidDisplay, HoledCapVisibleEdgesRetainIncidentFaceIdentity)
{
 sketch::Sketch sketch;(void)sketch.addRectangle({-5,-5},{5,5});
 (void)sketch.addCircle({{0,0},2});
 const auto extrusion=features::extrudeRegions(features::extractPlanarRegions(sketch),sketch.plane(),3);
 ASSERT_TRUE(extrusion.succeeded());
 const auto display=presentation::deriveSolidDisplay(extrusion.solids.front(),features::FeatureId{9},{0});
 const auto circularEdges=std::count_if(display.edges.begin(),display.edges.end(),[](const auto& edge){
  return edge.points.size()>2&&edge.incidentFaces.size()==2;
 });
 EXPECT_GT(circularEdges,0);
}
TEST(SolidDisplay, MultiHoleTopCapKeepsFullAreaAfterPlateWidthEdit)
{
 const sketch::SketchPlane basePlane;
 const sketch::SketchPlane topPlane{{0,0,8},{1,0,0},{0,1,0}};
 sketch::Sketch pocket;(void)pocket.addRectangle({7,-7},{25,7});
 for(const double halfWidth:{40.0,45.0}){
  sketch::Sketch base;(void)base.addRectangle({-halfWidth,-25},{halfWidth,25});
  (void)base.addCircle({{-22,0},4});
  const auto cut=features::subtractAxialRegions(features::extractPlanarRegions(base),basePlane,8,
   features::extractPlanarRegions(pocket),topPlane,{0,0,-1},2);
  ASSERT_EQ(cut.status,features::AxialSubtractionStatus::Success)<<cut.diagnostic;
  ASSERT_EQ(cut.solids.size(),1u);
  const auto display=presentation::deriveSolidDisplay(cut.solids.front(),features::FeatureId{9},{0});
  double topArea=0,bottomArea=0,floorArea=0;
  for(const auto&triangle:display.triangles){
   const auto&a=triangle.points[0],&b=triangle.points[1],&c=triangle.points[2];
   if(std::abs(a.z()-b.z())>1e-9||std::abs(a.z()-c.z())>1e-9)continue;
   const auto area=math::cross(b-a,c-a).length()/2;
   if(std::abs(a.z()-8)<1e-9)topArea+=area;
   else if(std::abs(a.z())<1e-9)bottomArea+=area;
   else if(std::abs(a.z()-6)<1e-9)floorArea+=area;
  }
  const auto tessellatedHoleArea=24.0*16.0*std::sin(std::numbers::pi/24.0);
  EXPECT_NEAR(bottomArea,2*halfWidth*50-tessellatedHoleArea,1e-6);
  EXPECT_NEAR(topArea,2*halfWidth*50-tessellatedHoleArea-18*14,1e-6);
  EXPECT_NEAR(floorArea,18*14,1e-6);
 }
}
}
