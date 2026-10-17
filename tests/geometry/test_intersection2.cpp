#include "core/geometry/Intersection2.h"
#include <gtest/gtest.h>
#include <numbers>
using namespace microsw::geometry;

TEST(Intersection2, SegmentCrossingTouchingAndOverlap)
{
    EXPECT_EQ(intersect(Segment2{{0,0},{2,2}},Segment2{{0,2},{2,0}}).kind,IntersectionKind::Proper);
    EXPECT_EQ(intersect(Segment2{{0,0},{1,0}},Segment2{{1,0},{2,0}}).kind,IntersectionKind::Endpoint);
    EXPECT_EQ(intersect(Segment2{{0,0},{2,0}},Segment2{{1,0},{3,0}}).kind,IntersectionKind::Overlap);
}
TEST(Intersection2, SegmentCircleClassifiesZeroOneTwo)
{
    const Circle2 c{{0,0},1};
    EXPECT_EQ(intersect(Segment2{{-2,2},{2,2}},c).kind,IntersectionKind::None);
    EXPECT_EQ(intersect(Segment2{{-2,1},{2,1}},c).kind,IntersectionKind::Tangent);
    EXPECT_EQ(intersect(Segment2{{-2,0},{2,0}},c).points.size(),2u);
}
TEST(Intersection2, CircleCircleClassifiesCases)
{
    const Circle2 a{{0,0},2};
    EXPECT_EQ(intersect(a,Circle2{{5,0},2}).kind,IntersectionKind::None);
    EXPECT_EQ(intersect(a,Circle2{{4,0},2}).kind,IntersectionKind::Tangent);
    EXPECT_EQ(intersect(a,Circle2{{2,0},2}).points.size(),2u);
    EXPECT_EQ(intersect(a,Circle2{{0,0},2}).kind,IntersectionKind::Coincident);
}
TEST(Intersection2, ArcFilteringAndArcOverlap)
{
    const Arc2 upper{{0,0},2,0,std::numbers::pi};
    EXPECT_EQ(intersect(Segment2{{0,-3},{0,3}},upper).points.size(),1u);
    const Arc2 right{{0,0},2,-std::numbers::pi/2,std::numbers::pi};
    EXPECT_EQ(intersect(upper,right).kind,IntersectionKind::Overlap);
    const Arc2 other{{2,0},2,std::numbers::pi/2,std::numbers::pi};
    EXPECT_TRUE(intersect(upper,other).intersects());
}
