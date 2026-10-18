#include "features/PlanarRegionBoolean.h"
#include <gtest/gtest.h>
#include <numbers>

using namespace microsw;

TEST(PlanarRegionBoolean, SplitsSegmentAndArcWithoutChangingCurveClass)
{
    auto pieces=features::detail::splitRegionCurve(geometry::Segment2{{0,0},{4,0}},{{1,0},{3,0}});
    ASSERT_EQ(pieces.size(),3u);for(const auto&piece:pieces)EXPECT_TRUE(std::holds_alternative<geometry::Segment2>(piece));
    const geometry::Arc2 arc{{0,0},2,0,std::numbers::pi};
    pieces=features::detail::splitRegionCurve(arc,{{0,2}});
    ASSERT_EQ(pieces.size(),2u);for(const auto&piece:pieces)EXPECT_TRUE(std::holds_alternative<geometry::Arc2>(piece));
}

TEST(PlanarRegionBoolean, RepresentsSplitCircleWithExactCircularArcs)
{
    const geometry::Circle2 circle{{0,0},2};
    const auto pieces=features::detail::splitRegionCurve(circle,{{2,0},{0,2},{-2,0},{0,-2}});
    ASSERT_EQ(pieces.size(),4u);
    for(const auto&piece:pieces){ASSERT_TRUE(std::holds_alternative<geometry::Arc2>(piece));EXPECT_NEAR(std::get<geometry::Arc2>(piece).radius(),2,1e-12);}
}
