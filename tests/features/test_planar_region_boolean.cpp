#include "features/PlanarRegionBoolean.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <numbers>

using namespace microsw;

namespace { features::PlanarRegionSet regions(sketch::Sketch s){return features::extractPlanarRegions(s);} sketch::Sketch rectangle(double a,double b,double c,double d){sketch::Sketch s;(void)s.addRectangle({a,b},{c,d});return s;} }

TEST(PlanarRegionBoolean, SplitsSegmentAndArcWithoutChangingCurveClass)
{
    auto pieces=features::detail::splitRegionCurve(geometry::Segment2{{0,0},{4,0}},{{1,0},{3,0}});
    ASSERT_EQ(pieces.size(),3u);for(const auto&piece:pieces)EXPECT_TRUE(std::holds_alternative<geometry::Segment2>(piece));
    const geometry::Arc2 arc{{0,0},2,0,std::numbers::pi};
    pieces=features::detail::splitRegionCurve(arc,{{0,2}});
    ASSERT_EQ(pieces.size(),2u);for(const auto&piece:pieces)EXPECT_TRUE(std::holds_alternative<geometry::Arc2>(piece));
}

TEST(PlanarRegionBoolean, SubtractsInnerRectangleAndCircleAsExactHoles)
{
    auto target=regions(rectangle(0,0,10,10));
    auto result=features::subtractPlanarRegions(target,regions(rectangle(2,2,4,4)));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);ASSERT_EQ(result.regions.regions.size(),1u);EXPECT_EQ(result.regions.regions[0].holes.size(),1u);
    sketch::Sketch circle;(void)circle.addCircle({{5,5},2});result=features::subtractPlanarRegions(target,regions(circle));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);ASSERT_EQ(result.regions.regions[0].holes.size(),1u);
    for(const auto&use:result.regions.regions[0].holes[0].uses)EXPECT_TRUE(std::holds_alternative<geometry::Arc2>(use.curve));
}

TEST(PlanarRegionBoolean, HandlesNoIntersectionFullRemovalAndSplit)
{
    auto target=regions(rectangle(0,0,10,4));
    auto disjoint=features::subtractPlanarRegions(target,regions(rectangle(20,20,21,21)));
    ASSERT_EQ(disjoint.status,features::RegionBooleanStatus::Success);EXPECT_EQ(disjoint.regions.regions.size(),1u);
    auto removed=features::subtractPlanarRegions(target,regions(rectangle(-1,-1,11,5)));
    EXPECT_EQ(removed.status,features::RegionBooleanStatus::Empty);
    auto split=features::subtractPlanarRegions(target,regions(rectangle(4,-1,6,5)));
    ASSERT_EQ(split.status,features::RegionBooleanStatus::Success);EXPECT_EQ(split.regions.regions.size(),2u);
}

TEST(PlanarRegionBoolean, RepresentsSplitCircleWithExactCircularArcs)
{
    const geometry::Circle2 circle{{0,0},2};
    const auto pieces=features::detail::splitRegionCurve(circle,{{2,0},{0,2},{-2,0},{0,-2}});
    ASSERT_EQ(pieces.size(),4u);
    for(const auto&piece:pieces){ASSERT_TRUE(std::holds_alternative<geometry::Arc2>(piece));EXPECT_NEAR(std::get<geometry::Arc2>(piece).radius(),2,1e-12);}
}

TEST(PlanarRegionBoolean, ClipsPartiallyOverlappingLinearAndCircularCutters)
{
    const auto target=regions(rectangle(0,0,10,10));
    auto result=features::subtractPlanarRegions(target,regions(rectangle(8,2,12,8)));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);
    ASSERT_EQ(result.regions.regions.size(),1u);
    EXPECT_TRUE(result.regions.regions.front().holes.empty());

    sketch::Sketch circle;(void)circle.addCircle({{10,5},2});
    result=features::subtractPlanarRegions(target,regions(circle));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);
    ASSERT_EQ(result.regions.regions.size(),1u);
    EXPECT_TRUE(result.regions.regions.front().holes.empty());
    EXPECT_TRUE(std::any_of(result.regions.regions.front().outer.uses.begin(),
        result.regions.regions.front().outer.uses.end(),[](const auto& use) {
            return std::holds_alternative<geometry::Arc2>(use.curve);
        }));
}

TEST(PlanarRegionBoolean, PreservesCutterVoidAndExistingTargetHole)
{
    auto targetSketch=rectangle(0,0,10,10);
    (void)targetSketch.addRectangle({1,1},{3,3});
    auto cutterSketch=rectangle(5,1,9,9);
    (void)cutterSketch.addRectangle({6,3},{8,7});
    const auto result=features::subtractPlanarRegions(
        regions(std::move(targetSketch)),regions(std::move(cutterSketch)));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);
    ASSERT_EQ(result.regions.regions.size(),2u);
}

TEST(PlanarRegionBoolean, HandlesConcaveTargetAndMultipleCuttingRegions)
{
    sketch::Sketch target;
    const auto a=target.addLine({{0,0},{8,0}});const auto b=target.addLine({{8,0},{8,3}});
    const auto c=target.addLine({{8,3},{3,3}});const auto d=target.addLine({{3,3},{3,8}});
    const auto e=target.addLine({{3,8},{0,8}});const auto f=target.addLine({{0,8},{0,0}});
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;
    auto cutter=rectangle(1,1,2,2);(void)cutter.addRectangle({5,1},{7,2});
    const auto result=features::subtractPlanarRegions(
        regions(std::move(target)),regions(std::move(cutter)));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);
    ASSERT_EQ(result.regions.regions.size(),1u);
    EXPECT_EQ(result.regions.regions.front().holes.size(),2u);
}

TEST(PlanarRegionBoolean, TangentialContactDoesNotCreateMicroRegions)
{
    sketch::Sketch circle;(void)circle.addCircle({{12,5},2});
    const auto result=features::subtractPlanarRegions(
        regions(rectangle(0,0,10,10)),regions(std::move(circle)));
    ASSERT_EQ(result.status,features::RegionBooleanStatus::Success);
    ASSERT_EQ(result.regions.regions.size(),1u);
    EXPECT_TRUE(result.regions.regions.front().holes.empty());
}
