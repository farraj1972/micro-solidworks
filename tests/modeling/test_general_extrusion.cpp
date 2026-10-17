#include "features/GeneralExtrusion.h"
#include "features/PlanarRegion.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <variant>

namespace
{
using namespace microsw;

features::GeneralExtrusionResult extrude(const sketch::Sketch& sketch, double distance = 3.0)
{
    return features::extrudeRegions(features::extractPlanarRegions(sketch),
                                    sketch.plane(), distance);
}

TEST(GeneralExtrusion, BuildsConcaveRegionTransactionally)
{
    sketch::Sketch sketch;
    for (const auto& segment : std::vector<geometry::Segment2>{{{0, 0}, {3, 0}},
             {{3, 0}, {3, 1}}, {{3, 1}, {1, 1}}, {{1, 1}, {1, 3}},
             {{1, 3}, {0, 3}}, {{0, 3}, {0, 0}}})
        (void)sketch.addLine(segment);

    const auto result = extrude(sketch);

    ASSERT_TRUE(result.succeeded()) << result.diagnostic;
    ASSERT_EQ(result.solids.size(), 1u);
    EXPECT_TRUE(result.solids.front().isValid());
    EXPECT_EQ(result.solids.front().faces().size(), 8u);
}

TEST(GeneralExtrusion, PreservesCircularEdgesAndCylindricalSupport)
{
    sketch::Sketch sketch;
    (void)sketch.addCircle({{1, 2}, 2});

    const auto result = extrude(sketch);

    ASSERT_TRUE(result.succeeded()) << result.diagnostic;
    const auto& solid = result.solids.front();
    EXPECT_EQ(solid.edges().size(), 6u);
    EXPECT_EQ(solid.faces().size(), 4u);
    EXPECT_EQ(std::count_if(solid.edges().begin(), solid.edges().end(), [](const auto& edge) {
        return std::holds_alternative<geometry::CircularArc3>(edge.geometry());
    }), 4);
    EXPECT_EQ(std::count_if(solid.faces().begin(), solid.faces().end(), [](const auto& face) {
        return std::holds_alternative<geometry::Cylinder>(face.support());
    }), 2);
}

TEST(GeneralExtrusion, BuildsOneSolidWithAThroughHole)
{
    sketch::Sketch sketch;
    (void)sketch.addRectangle({-5, -5}, {5, 5});
    (void)sketch.addCircle({{0, 0}, 2});

    const auto result = extrude(sketch);

    ASSERT_TRUE(result.succeeded()) << result.diagnostic;
    ASSERT_EQ(result.solids.size(), 1u);
    const auto& solid = result.solids.front();
    ASSERT_TRUE(solid.isValid());
    ASSERT_GE(solid.faces().size(), 2u);
    EXPECT_EQ(solid.faces()[solid.rootShell().uses()[0].face.value()].innerWires().size(), 1u);
    EXPECT_EQ(std::count_if(solid.faces().begin(), solid.faces().end(), [](const auto& face) {
        return std::holds_alternative<geometry::Cylinder>(face.support());
    }), 2);
}

TEST(GeneralExtrusion, CreatesIndependentSolidsForDisjointRegions)
{
    sketch::Sketch sketch;
    (void)sketch.addCircle({{-3, 0}, 1});
    (void)sketch.addCircle({{3, 0}, 1});

    const auto result = extrude(sketch);

    ASSERT_TRUE(result.succeeded()) << result.diagnostic;
    ASSERT_EQ(result.solids.size(), 2u);
    EXPECT_TRUE(result.solids[0].isValid());
    EXPECT_TRUE(result.solids[1].isValid());
}

TEST(GeneralExtrusion, RejectsInvalidInputWithoutPartialSolids)
{
    sketch::Sketch open;
    (void)open.addLine({{0, 0}, {1, 0}});
    auto result = extrude(open);
    EXPECT_EQ(result.status, features::GeneralExtrusionStatus::InvalidRegions);
    EXPECT_TRUE(result.solids.empty());

    sketch::Sketch circle;
    (void)circle.addCircle({{0, 0}, 1});
    result = extrude(circle, 0);
    EXPECT_EQ(result.status, features::GeneralExtrusionStatus::InvalidDistance);
    EXPECT_TRUE(result.solids.empty());
}
}
