#include "features/PlanarRegion.h"
#include <gtest/gtest.h>
#include <numbers>
using namespace microsw;

TEST(PlanarRegion, RectangleCircleAndConcaveL)
{
    sketch::Sketch rectangle; (void)rectangle.addRectangle({0,0},{4,3});
    auto r=features::extractPlanarRegions(rectangle);ASSERT_TRUE(r.valid());EXPECT_EQ(r.regions.size(),1u);
    sketch::Sketch circle;circle.addCircle({{0,0},2});
    r=features::extractPlanarRegions(circle);ASSERT_TRUE(r.valid());EXPECT_EQ(r.regions[0].outer.uses.size(),1u);
    sketch::Sketch l;for(const auto&s:std::vector<geometry::Segment2>{{{0,0},{3,0}},{{3,0},{3,1}},{{3,1},{1,1}},{{1,1},{1,3}},{{1,3},{0,3}},{{0,3},{0,0}}})l.addLine(s);
    r=features::extractPlanarRegions(l);EXPECT_TRUE(r.valid());EXPECT_EQ(r.regions.size(),1u);
}
TEST(PlanarRegion, ArcLineLoopIsExact)
{
    sketch::Sketch s;s.addArc({{0,0},2,0,std::numbers::pi});s.addLine({{-2,0},{2,0}});
    const auto r=features::extractPlanarRegions(s);ASSERT_TRUE(r.valid());EXPECT_EQ(r.regions[0].outer.uses.size(),2u);
}
TEST(PlanarRegion, HolesDisjointAndNestedIslandUseParity)
{
    sketch::Sketch hole;(void)hole.addRectangle({-5,-5},{5,5});hole.addCircle({{0,0},2});
    auto r=features::extractPlanarRegions(hole);ASSERT_TRUE(r.valid());ASSERT_EQ(r.regions.size(),1u);EXPECT_EQ(r.regions[0].holes.size(),1u);
    sketch::Sketch disjoint;disjoint.addCircle({{-3,0},1});disjoint.addCircle({{3,0},1});
    r=features::extractPlanarRegions(disjoint);ASSERT_TRUE(r.valid());EXPECT_EQ(r.regions.size(),2u);
    sketch::Sketch nested;nested.addCircle({{0,0},5});nested.addCircle({{0,0},3});nested.addCircle({{0,0},1});
    r=features::extractPlanarRegions(nested);ASSERT_TRUE(r.valid());EXPECT_EQ(r.regions.size(),2u);EXPECT_EQ(r.regions[0].holes.size(),1u);
}
TEST(PlanarRegion, RejectsOpenBranchSelfIntersectionOverlapAndTangent)
{
    sketch::Sketch open;open.addLine({{0,0},{1,0}});EXPECT_EQ(features::extractPlanarRegions(open).diagnostic.status,features::RegionStatus::OpenContour);
    sketch::Sketch branch;(void)branch.addRectangle({0,0},{2,2});branch.addLine({{0,0},{-1,0}});EXPECT_EQ(features::extractPlanarRegions(branch).diagnostic.status,features::RegionStatus::Branch);
    sketch::Sketch bow;bow.addLine({{0,0},{2,2}});bow.addLine({{2,2},{0,2}});bow.addLine({{0,2},{2,0}});bow.addLine({{2,0},{0,0}});EXPECT_EQ(features::extractPlanarRegions(bow).diagnostic.status,features::RegionStatus::SelfIntersection);
    sketch::Sketch duplicate;duplicate.addCircle({{0,0},2});duplicate.addCircle({{0,0},2});EXPECT_EQ(features::extractPlanarRegions(duplicate).diagnostic.status,features::RegionStatus::Overlap);
    sketch::Sketch tangent;tangent.addCircle({{0,0},2});tangent.addCircle({{4,0},2});EXPECT_EQ(features::extractPlanarRegions(tangent).diagnostic.status,features::RegionStatus::Ambiguous);
}
