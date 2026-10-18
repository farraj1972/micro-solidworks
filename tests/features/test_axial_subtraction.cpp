#include "features/AxialSubtraction.h"
#include "features/PlanarRegion.h"

#include <gtest/gtest.h>
#include <algorithm>

using namespace microsw;

namespace
{
features::PlanarRegionSet rectangle(double x0,double y0,double x1,double y1)
{
    sketch::Sketch value;(void)value.addRectangle({x0,y0},{x1,y1});
    return features::extractPlanarRegions(value);
}

sketch::SketchPlane planeAt(double z)
{
    return {{0,0,z},{1,0,0},{0,1,0}};
}
}

TEST(AxialSubtraction, BuildsExactRectangularBlindPocket)
{
    const auto result=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        rectangle(2,2,8,8),planeAt(10),{0,0,-1},4);
    ASSERT_EQ(result.status,features::AxialSubtractionStatus::Success) << result.diagnostic;
    ASSERT_EQ(result.solids.size(),1u);
    EXPECT_TRUE(result.solids.front().isValid());
}

TEST(AxialSubtraction, BuildsExactCircularBlindHole)
{
    sketch::Sketch circle;(void)circle.addCircle({{5,5},2});
    const auto result=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        features::extractPlanarRegions(circle),planeAt(10),{0,0,-1},4);
    ASSERT_EQ(result.status,features::AxialSubtractionStatus::Success) << result.diagnostic;
    ASSERT_EQ(result.solids.size(),1u);
    EXPECT_TRUE(std::any_of(result.solids.front().faces().begin(),result.solids.front().faces().end(),
        [](const auto&face){return std::holds_alternative<geometry::Cylinder>(face.support());}));
}

TEST(AxialSubtraction, SupportsFullDepthSplitAndEmptyResults)
{
    auto split=features::subtractAxialRegions(rectangle(0,0,10,4),planeAt(0),10,
        rectangle(4,-1,6,5),planeAt(10),{0,0,-1},20);
    ASSERT_EQ(split.status,features::AxialSubtractionStatus::Success) << split.diagnostic;
    EXPECT_EQ(split.solids.size(),2u);
    auto empty=features::subtractAxialRegions(rectangle(0,0,10,4),planeAt(0),10,
        rectangle(-1,-1,11,5),planeAt(10),{0,0,-1},20);
    EXPECT_EQ(empty.status,features::AxialSubtractionStatus::SuccessEmpty);
}

TEST(AxialSubtraction, ReportsNoIntersectionAndSupportsReverseBlindCut)
{
    auto none=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        rectangle(20,20,21,21),planeAt(10),{0,0,-1},4);
    EXPECT_EQ(none.status,features::AxialSubtractionStatus::NoIntersection);
    auto reverse=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        rectangle(2,2,8,8),planeAt(0),{0,0,1},4);
    ASSERT_EQ(reverse.status,features::AxialSubtractionStatus::Success) << reverse.diagnostic;
    EXPECT_TRUE(reverse.solids.front().isValid());
}

TEST(AxialSubtraction, SupportsConcaveVoidAndMultipleCuttingRegions)
{
    sketch::Sketch cutter;
    (void)cutter.addLine({{1,1},{8,1}});(void)cutter.addLine({{8,1},{8,3}});
    (void)cutter.addLine({{8,3},{3,3}});(void)cutter.addLine({{3,3},{3,8}});
    (void)cutter.addLine({{3,8},{1,8}});(void)cutter.addLine({{1,8},{1,1}});
    auto concave=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        features::extractPlanarRegions(cutter),planeAt(10),{0,0,-1},3);
    ASSERT_EQ(concave.status,features::AxialSubtractionStatus::Success) << concave.diagnostic;

    sketch::Sketch annulusSketch;(void)annulusSketch.addRectangle({2,2},{8,8});
    (void)annulusSketch.addRectangle({4,4},{6,6});
    auto withVoid=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        features::extractPlanarRegions(annulusSketch),planeAt(10),{0,0,-1},3);
    ASSERT_EQ(withVoid.status,features::AxialSubtractionStatus::Success) << withVoid.diagnostic;

    sketch::Sketch multiple;(void)multiple.addCircle({{3,5},1});(void)multiple.addCircle({{7,5},1});
    auto twoHoles=features::subtractAxialRegions(rectangle(0,0,10,10),planeAt(0),10,
        features::extractPlanarRegions(multiple),planeAt(10),{0,0,-1},3);
    ASSERT_EQ(twoHoles.status,features::AxialSubtractionStatus::Success) << twoHoles.diagnostic;
}

TEST(AxialSubtraction, ProcessesOnlyIntersectedBodiesInMultiBodyTarget)
{
    sketch::Sketch target;(void)target.addRectangle({0,0},{4,4});(void)target.addRectangle({10,0},{14,4});
    const auto result=features::subtractAxialRegions(features::extractPlanarRegions(target),planeAt(0),8,
        rectangle(1,1,3,3),planeAt(8),{0,0,-1},2);
    ASSERT_EQ(result.status,features::AxialSubtractionStatus::Success) << result.diagnostic;
    ASSERT_EQ(result.solids.size(),2u);
    EXPECT_TRUE(result.solids[0].isValid());EXPECT_TRUE(result.solids[1].isValid());
}
