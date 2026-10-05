#include "modeling/Boolean.h"
#include "modeling/Extrusion.h"
#include "modeling/Profile.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
namespace {
using namespace microsw;
topology::Solid box(double x0,double y0,double z0,double x1,double y1,double z1){
 return modeling::extrude(modeling::Profile{{{x0,y0,z0},{x1,y0,z0},{x1,y1,z0},{x0,y1,z0}},{{x0,y0,z0},{0,0,1}}},z1-z0);
}
std::array<double,6> bounds(const topology::Solid&s){std::array<double,6>v{1e9,1e9,1e9,-1e9,-1e9,-1e9};for(const auto&x:s.vertices()){const auto&p=x.point();v[0]=std::min(v[0],p.x());v[1]=std::min(v[1],p.y());v[2]=std::min(v[2],p.z());v[3]=std::max(v[3],p.x());v[4]=std::max(v[4],p.y());v[5]=std::max(v[5],p.z());}return v;}
TEST(Boolean, IntersectionProducesExpectedBoxAndEmptyWhenDisjoint){
 auto a=box(0,0,0,4,4,4),b=box(2,1,1,6,3,5);auto r=modeling::booleanOperation(a,b,modeling::BooleanOperation::Intersection);
 ASSERT_EQ(r.status(),modeling::BooleanStatus::SUCCESS);ASSERT_TRUE(r.solid());EXPECT_EQ(bounds(*r.solid()),(std::array<double,6>{2,1,1,4,3,4}));
 auto d=box(10,10,10,12,12,12);EXPECT_EQ(modeling::booleanOperation(a,d,modeling::BooleanOperation::Intersection).status(),modeling::BooleanStatus::EMPTY);
}
TEST(Boolean, UnionProducesConnectedNonConvexManifold){
 auto a=box(0,0,0,4,4,4),b=box(2,2,1,6,6,5);auto r=modeling::booleanOperation(a,b,modeling::BooleanOperation::Union);
 ASSERT_EQ(r.status(),modeling::BooleanStatus::SUCCESS);ASSERT_TRUE(r.solid());EXPECT_TRUE(r.solid()->isValid());EXPECT_GT(r.solid()->faces().size(),6u);
}
TEST(Boolean, DifferenceSupportsCornerCutContainmentAndDisjoint){
 auto a=box(0,0,0,6,6,6),cut=box(3,3,3,8,8,8);auto r=modeling::booleanOperation(a,cut,modeling::BooleanOperation::Difference);
 ASSERT_EQ(r.status(),modeling::BooleanStatus::SUCCESS);ASSERT_TRUE(r.solid());EXPECT_GT(r.solid()->faces().size(),6u);
 auto containing=box(-1,-1,-1,7,7,7);EXPECT_EQ(modeling::booleanOperation(a,containing,modeling::BooleanOperation::Difference).status(),modeling::BooleanStatus::EMPTY);
 auto disjoint=box(10,10,10,12,12,12);EXPECT_EQ(modeling::booleanOperation(a,disjoint,modeling::BooleanOperation::Difference).status(),modeling::BooleanStatus::SUCCESS);
}
TEST(Boolean, RejectsContactAmbiguityAndUnsupportedInput){
 auto a=box(0,0,0,2,2,2);for(auto b:{box(2,0,0,4,2,2),box(2,2,0,4,4,2),box(2,2,2,4,4,4)})
  EXPECT_EQ(modeling::booleanOperation(a,b,modeling::BooleanOperation::Union).status(),modeling::BooleanStatus::UNSUPPORTED_CASE);
 topology::Solid invalid;EXPECT_EQ(modeling::booleanOperation(invalid,a,modeling::BooleanOperation::Union).status(),modeling::BooleanStatus::INVALID_INPUT);
}
TEST(Boolean, RejectsCavityAndThroughHoleDifference){
 auto a=box(0,0,0,10,10,10);
 EXPECT_EQ(modeling::booleanOperation(a,box(2,2,2,8,8,8),modeling::BooleanOperation::Difference).status(),modeling::BooleanStatus::UNSUPPORTED_CASE);
 EXPECT_EQ(modeling::booleanOperation(a,box(2,2,-1,8,8,11),modeling::BooleanOperation::Difference).status(),modeling::BooleanStatus::UNSUPPORTED_CASE);
}
TEST(Boolean, InputsRemainUnchangedAndFailureCarriesNoSolid){
 auto a=box(0,0,0,4,4,4),b=box(2,2,2,6,6,6);auto av=a.vertices(),bv=b.vertices();auto r=modeling::booleanOperation(a,b,modeling::BooleanOperation::Difference);
 EXPECT_EQ(a.vertices().size(),av.size());EXPECT_EQ(b.vertices().size(),bv.size());ASSERT_EQ(r.status(),modeling::BooleanStatus::SUCCESS);
 auto unsupported=modeling::booleanOperation(a,box(4,0,0,8,4,4),modeling::BooleanOperation::Union);EXPECT_FALSE(unsupported.solid());
}
}
