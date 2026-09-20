#include "app/modeling/SketchProfileAdapter.h"
#include "modeling/Extrusion.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <limits>

namespace
{
using namespace microsw;

TEST(Profile, ValidatesAndDerivesSegments)
{
    modeling::Profile profile{{{0,0,0},{3,0,0},{3,2,0},{0,2,0}}, {{0,0,0},{0,0,1}}};
    EXPECT_EQ(profile.boundary().size(), 4u);
    EXPECT_EQ(profile.segments().size(), 4u);
    EXPECT_THROW((modeling::Profile{{{0,0,0},{0,2,0},{3,2,0},{3,0,0}}, {{0,0,0},{0,0,1}}}), std::invalid_argument);
    EXPECT_THROW((modeling::Profile{{{0,0,0},{2,0,0},{1,0.5,0},{2,2,0},{0,2,0}}, {{0,0,0},{0,0,1}}}), std::invalid_argument);
    EXPECT_THROW((modeling::Profile{{{0,0,0},{1,0,0},{1,1,1}}, {{0,0,0},{0,0,1}}}), std::invalid_argument);
}

TEST(Extrusion, BuildsSharedManifoldForTriangleRectangleAndPentagon)
{
    for (const auto& points : std::vector<std::vector<geometry::Point3>>{
        {{0,0,0},{2,0,0},{0,2,0}},
        {{0,0,0},{4,0,0},{4,3,0},{0,3,0}},
        {{0,0,0},{2,0,0},{3,1,0},{1.5,3,0},{0,1,0}}})
    {
        const modeling::Profile profile{points, {{0,0,0},{0,0,1}}};
        const auto solid = modeling::extrude(profile, 5);
        const auto n = points.size();
        EXPECT_TRUE(solid.isValid());
        EXPECT_EQ(solid.vertices().size(), 2*n);
        EXPECT_EQ(solid.edges().size(), 3*n);
        EXPECT_EQ(solid.faces().size(), n+2);
        EXPECT_EQ(solid.shells().size(), 1u);
    }
}

TEST(Extrusion, RectangleHasExpectedBoundsAndRejectsInvalidDistance)
{
    const modeling::Profile profile{{{1,2,3},{5,2,3},{5,8,3},{1,8,3}}, {{1,2,3},{0,0,1}}};
    const auto solid = modeling::extrude(profile, 7);
    double minX=1e9,maxX=-1e9,minY=1e9,maxY=-1e9,minZ=1e9,maxZ=-1e9;
    for (const auto& vertex : solid.vertices()) { const auto&p=vertex.point(); minX=std::min(minX,p.x());maxX=std::max(maxX,p.x());minY=std::min(minY,p.y());maxY=std::max(maxY,p.y());minZ=std::min(minZ,p.z());maxZ=std::max(maxZ,p.z()); }
    EXPECT_DOUBLE_EQ(maxX-minX,4); EXPECT_DOUBLE_EQ(maxY-minY,6); EXPECT_DOUBLE_EQ(maxZ-minZ,7);
    EXPECT_THROW((void)modeling::extrude(profile, 0), std::invalid_argument);
    EXPECT_THROW((void)modeling::extrude(profile, -1), std::invalid_argument);
    EXPECT_THROW((void)modeling::extrude(profile, std::numeric_limits<double>::infinity()), std::invalid_argument);
}

TEST(SketchProfileAdapter, OrdersUnorderedRectangleAndRejectsInvalidGraphs)
{
    sketch::Sketch rectangle;
    rectangle.addLine({{4,3},{0,3}}); rectangle.addLine({{0,0},{4,0}});
    rectangle.addLine({{0,3},{0,0}}); rectangle.addLine({{4,0},{4,3}});
    const auto profile = extractProfile(rectangle);
    EXPECT_EQ(profile.boundary().size(),4u);
    EXPECT_NO_THROW((void)modeling::extrude(profile,2));

    sketch::Sketch open; open.addLine({{0,0},{1,0}}); open.addLine({{1,0},{1,1}}); open.addLine({{1,1},{2,1}});
    EXPECT_THROW((void)extractProfile(open), std::invalid_argument);
    sketch::Sketch branching; branching.addLine({{0,0},{1,0}}); branching.addLine({{1,0},{1,1}}); branching.addLine({{1,0},{2,0}});
    EXPECT_THROW((void)extractProfile(branching), std::invalid_argument);
    sketch::Sketch disconnected;
    disconnected.addLine({{0,0},{1,0}}); disconnected.addLine({{1,0},{0,1}});
    disconnected.addLine({{0,1},{0,0}}); disconnected.addLine({{4,4},{5,4}});
    EXPECT_THROW((void)extractProfile(disconnected), std::invalid_argument);
    sketch::Sketch nonConvex;
    nonConvex.addLine({{0,0},{3,0}}); nonConvex.addLine({{3,0},{1,1}});
    nonConvex.addLine({{1,1},{3,3}}); nonConvex.addLine({{3,3},{0,3}});
    nonConvex.addLine({{0,3},{0,0}});
    EXPECT_THROW((void)extractProfile(nonConvex), std::invalid_argument);
}

TEST(SketchProfileAdapter, MatchesEndpointsWithinGeometricTolerance)
{
    constexpr double epsilon = geometry::defaultGeometricTolerance * 0.25;
    sketch::Sketch sketch;
    sketch.addLine({{0,0},{4,0}});
    sketch.addLine({{4 + epsilon,0},{4,3}});
    sketch.addLine({{4,3 + epsilon},{0,3}});
    sketch.addLine({{-epsilon,3},{0,epsilon}});
    const auto profile = extractProfile(sketch);
    EXPECT_EQ(profile.boundary().size(), 4u);
    EXPECT_TRUE(modeling::extrude(profile, 2).isValid());
}
}
