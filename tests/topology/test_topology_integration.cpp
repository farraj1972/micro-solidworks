#include "core/topology/CuboidBuilder.h"

#include <gtest/gtest.h>
#include <array>
#include <map>
#include <limits>
#include <set>

namespace
{
using namespace microsw;
using namespace microsw::topology;

TEST(TopologyCuboid, BuildsDeterministicSharedClosedSolid)
{
    const auto solid = makeCuboid(2,4,6);
    ASSERT_TRUE(solid.isValid());
    EXPECT_EQ(solid.vertices().size(), 8u);
    EXPECT_EQ(solid.edges().size(), 12u);
    EXPECT_EQ(solid.wires().size(), 6u);
    EXPECT_EQ(solid.faces().size(), 6u);
    EXPECT_EQ(solid.shells().size(), 1u);
    ASSERT_TRUE(solid.rootShellId().has_value());
    EXPECT_EQ(solid.rootShellId()->value(), 0u);
    EXPECT_EQ(solid.rootShell().uses().size(), 6u);
    EXPECT_EQ(solid.rootShell().uses().front().orientation, FaceOrientation::Reversed);

    std::map<std::uint32_t,int> edgeUses;
    std::map<std::uint32_t,int> vertexUses;
    for (const auto& edge : solid.edges())
    {
        ++vertexUses[edge.start().value()];
        ++vertexUses[edge.end().value()];
        EXPECT_FALSE(solid.segment(EdgeId{static_cast<std::uint32_t>(&edge - solid.edges().data())}).isDegenerate());
    }
    for (const auto& face : solid.faces())
        for (const auto& use : solid.find(face.outerWire()).uses()) ++edgeUses[use.edge.value()];
    EXPECT_EQ(edgeUses.size(), 12u);
    for (const auto& [edge,count] : edgeUses) { (void)edge; EXPECT_EQ(count, 2); }
    for (const auto& [vertex,count] : vertexUses) { (void)vertex; EXPECT_EQ(count, 3); }

    const geometry::Point3 center{0,0,0};
    for (const auto& use : solid.rootShell().uses())
    {
        const auto& plane = solid.find(use.face).supportPlane();
        const auto normal = use.orientation == FaceOrientation::Forward
            ? plane.normal() : -plane.normal();
        EXPECT_LT(math::dot(center - plane.origin(), normal), 0.0);
    }
}

TEST(TopologyCuboid, PreservesCanonicalAndReversedEdgeUses)
{
    const auto solid = makeCuboid(2,2,2);
    bool sawForward = false;
    bool sawReversed = false;
    for (const auto& wire : solid.wires()) for (const auto& use : wire.uses())
    {
        sawForward |= use.orientation == EdgeOrientation::Forward;
        sawReversed |= use.orientation == EdgeOrientation::Reversed;
    }
    EXPECT_TRUE(sawForward);
    EXPECT_TRUE(sawReversed);
}

TEST(TopologyCuboid, RejectsInvalidDimensions)
{
    EXPECT_THROW((void)makeCuboid(0,1,1), std::invalid_argument);
    EXPECT_THROW((void)makeCuboid(-1,1,1), std::invalid_argument);
    EXPECT_THROW((void)makeCuboid(1,1,std::numeric_limits<double>::infinity()), std::invalid_argument);
}

TEST(TopologyShell, RejectsOpenAndIncoherentFaceSets)
{
    Solid solid;
    const std::array<VertexId,4> v{solid.addVertex({0,0,0}),solid.addVertex({1,0,0}),
        solid.addVertex({0,1,0}),solid.addVertex({0,0,1})};
    const std::array<EdgeId,6> e{solid.addEdge(v[0],v[1]),solid.addEdge(v[1],v[2]),
        solid.addEdge(v[2],v[0]),solid.addEdge(v[0],v[3]),solid.addEdge(v[1],v[3]),solid.addEdge(v[2],v[3])};
    const auto w0=solid.addWire({{e[0],EdgeOrientation::Forward},{e[1],EdgeOrientation::Forward},{e[2],EdgeOrientation::Forward}});
    const auto w1=solid.addWire({{e[3],EdgeOrientation::Forward},{e[4],EdgeOrientation::Reversed},{e[0],EdgeOrientation::Reversed}});
    const auto f0=solid.addFace(w0,{{0,0,0},{0,0,1}});
    const auto f1=solid.addFace(w1,{{0,0,0},{0,1,0}});
    EXPECT_THROW(solid.addShell({{f0,FaceOrientation::Forward}}), std::invalid_argument);
    EXPECT_THROW(solid.addShell({{FaceId{},FaceOrientation::Forward}}), std::out_of_range);
    EXPECT_THROW(solid.addShell({{f0,FaceOrientation::Forward},{f0,FaceOrientation::Reversed}}), std::invalid_argument);
    EXPECT_THROW(solid.addShell({{f0,FaceOrientation::Forward},{f1,FaceOrientation::Reversed}}), std::invalid_argument);
}

TEST(TopologyShell, RejectsAnEdgeUsedByMoreThanTwoFaces)
{
    Solid solid;
    const auto a=solid.addVertex({0,0,0}); const auto b=solid.addVertex({1,0,0});
    const auto c=solid.addVertex({0,1,0}); const auto d=solid.addVertex({0,0,1});
    const auto e=solid.addVertex({0.5,1,1});
    const auto common=solid.addEdge(a,b);
    const auto ac=solid.addEdge(a,c); const auto bc=solid.addEdge(b,c);
    const auto ad=solid.addEdge(a,d); const auto bd=solid.addEdge(b,d);
    const auto ae=solid.addEdge(a,e); const auto be=solid.addEdge(b,e);
    const auto w0=solid.addWire({{common,EdgeOrientation::Forward},{bc,EdgeOrientation::Forward},{ac,EdgeOrientation::Reversed}});
    const auto w1=solid.addWire({{common,EdgeOrientation::Forward},{bd,EdgeOrientation::Forward},{ad,EdgeOrientation::Reversed}});
    const auto w2=solid.addWire({{common,EdgeOrientation::Forward},{be,EdgeOrientation::Forward},{ae,EdgeOrientation::Reversed}});
    const auto f0=solid.addFace(w0,{{0,0,0},{0,0,1}});
    const auto f1=solid.addFace(w1,{{0,0,0},{0,-1,0}});
    const auto f2=solid.addFace(w2,{{0,0,0},{0,-1,1}});
    EXPECT_THROW((void)solid.addShell({{f0,FaceOrientation::Forward},{f1,FaceOrientation::Forward},
        {f2,FaceOrientation::Forward}}), std::invalid_argument);
}

TEST(TopologyInspection, CopiesAndMovesPreserveAggregateLocalRelations)
{
    const auto original = makeCuboid(2,2,2);
    auto copy = original;
    auto moved = std::move(copy);
    ASSERT_TRUE(moved.isValid());
    EXPECT_EQ(moved.find(EdgeId{0}).start(), original.find(EdgeId{0}).start());
    EXPECT_EQ(moved.find(FaceId{0}).outerWire(), original.find(FaceId{0}).outerWire());
}

}
