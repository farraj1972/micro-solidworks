#include "app/modeling/ActiveBoolean.h"
#include "modeling/Extrusion.h"
#include "modeling/Profile.h"
#include <gtest/gtest.h>
namespace { using namespace microsw;
topology::Solid appBox(double x0,double y0,double z0,double x1,double y1,double z1){return modeling::extrude(modeling::Profile{{{x0,y0,z0},{x1,y0,z0},{x1,y1,z0},{x0,y1,z0}},{{x0,y0,z0},{0,0,1}}},z1-z0);}
TEST(BooleanVerticalSlice, PublishesSelectableResultAndPreservesItOnFailure){
 ActiveBoolean active;EXPECT_EQ(active.execute(modeling::BooleanOperation::Union),modeling::BooleanStatus::INVALID_INPUT);
 active.setOperandA(appBox(0,0,0,4,4,4));active.setOperandB(appBox(2,2,1,6,6,5));
 ASSERT_EQ(active.execute(modeling::BooleanOperation::Union),modeling::BooleanStatus::SUCCESS);ASSERT_NE(active.solid(),nullptr);ASSERT_TRUE(active.presentation().visualId());auto id=*active.presentation().visualId();
 active.setOperandB(appBox(4,0,0,8,4,4));EXPECT_EQ(active.execute(modeling::BooleanOperation::Union),modeling::BooleanStatus::UNSUPPORTED_CASE);ASSERT_NE(active.solid(),nullptr);EXPECT_EQ(*active.presentation().visualId(),id);
}
TEST(BooleanVerticalSlice, EmptyCommitsLogicalEmptyResult){
 ActiveBoolean active;active.setOperandA(appBox(0,0,0,2,2,2));active.setOperandB(appBox(4,4,4,6,6,6));
 EXPECT_EQ(active.execute(modeling::BooleanOperation::Intersection),modeling::BooleanStatus::EMPTY);EXPECT_EQ(active.solid(),nullptr);
}
}
