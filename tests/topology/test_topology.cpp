#include "core/topology/Solid.h"

#include <gtest/gtest.h>
#include <array>
#include <numbers>
#include <type_traits>
#include <variant>

namespace
{
using namespace microsw;
using namespace microsw::topology;

TEST(TopologyIdentity, IsStrongLocalIndexIdentity)
{
    static_assert(!std::is_same_v<VertexId, EdgeId>);
    static_assert(!std::is_convertible_v<VertexId, EdgeId>);
    EXPECT_FALSE(VertexId{}.isValid());
    EXPECT_EQ(VertexId{}.value(), UINT32_MAX);
    EXPECT_NE(VertexId{0}, VertexId{1});
}

TEST(TopologyVertexEdge, OwnsPointsAndDerivesSegments)
{
    Solid solid;
    const auto a = solid.addVertex({1,2,3});
    const auto duplicatePosition = solid.addVertex({1,2,3});
    const auto b = solid.addVertex({4,6,3});
    EXPECT_NE(a, duplicatePosition);
    const auto edge = solid.addEdge(a, b);
    EXPECT_EQ(solid.find(edge).start(), a);
    EXPECT_EQ(solid.find(edge).end(), b);
    EXPECT_DOUBLE_EQ(solid.segment(edge).length(), 5.0);
    EXPECT_THROW(solid.addEdge(a, a), std::invalid_argument);
    EXPECT_THROW(solid.addEdge(a, duplicatePosition), std::invalid_argument);
    EXPECT_THROW(solid.addEdge(VertexId{}, b), std::out_of_range);
    EXPECT_THROW((void)solid.find(EdgeId{}), std::out_of_range);
}

struct Square
{
    Solid solid;
    std::array<VertexId,4> vertices;
    std::array<EdgeId,4> edges;

    Square()
    {
        vertices = {solid.addVertex({0,0,0}), solid.addVertex({1,0,0}),
                    solid.addVertex({1,1,0}), solid.addVertex({0,1,0})};
        edges = {solid.addEdge(vertices[0],vertices[1]), solid.addEdge(vertices[1],vertices[2]),
                 solid.addEdge(vertices[2],vertices[3]), solid.addEdge(vertices[3],vertices[0])};
    }
};

TEST(TopologyWire, RequiresOrderedClosedUniqueTraversal)
{
    Square value;
    const auto wire = value.solid.addWire({{value.edges[0],EdgeOrientation::Forward},
        {value.edges[1],EdgeOrientation::Forward},{value.edges[2],EdgeOrientation::Forward},
        {value.edges[3],EdgeOrientation::Forward}});
    EXPECT_EQ(value.solid.find(wire).uses().size(), 4u);
    EXPECT_THROW(value.solid.addWire({{value.edges[0],EdgeOrientation::Forward},
        {value.edges[2],EdgeOrientation::Forward},{value.edges[1],EdgeOrientation::Forward}}), std::invalid_argument);
    EXPECT_THROW(value.solid.addWire({{value.edges[0],EdgeOrientation::Forward},
        {value.edges[1],EdgeOrientation::Forward},{value.edges[0],EdgeOrientation::Reversed}}), std::invalid_argument);
    EXPECT_THROW(value.solid.addWire({{EdgeId{},EdgeOrientation::Forward},
        {value.edges[1],EdgeOrientation::Forward},{value.edges[2],EdgeOrientation::Forward}}), std::out_of_range);
}

TEST(TopologyFace, RequiresCoplanarConvexCounterClockwiseBoundary)
{
    Square value;
    const auto ccw = value.solid.addWire({{value.edges[0],EdgeOrientation::Forward},
        {value.edges[1],EdgeOrientation::Forward},{value.edges[2],EdgeOrientation::Forward},
        {value.edges[3],EdgeOrientation::Forward}});
    const geometry::Plane plane{{0,0,0},{0,0,1}};
    const auto face = value.solid.addFace(ccw, plane);
    EXPECT_EQ(value.solid.find(face).outerWire(), ccw);

    const auto clockwise = value.solid.addWire({{value.edges[3],EdgeOrientation::Reversed},
        {value.edges[2],EdgeOrientation::Reversed},{value.edges[1],EdgeOrientation::Reversed},
        {value.edges[0],EdgeOrientation::Reversed}});
    EXPECT_THROW(value.solid.addFace(clockwise, plane), std::invalid_argument);
    EXPECT_THROW(value.solid.addFace(ccw, {{0,0,1},{0,0,1}}), std::invalid_argument);
    EXPECT_THROW(value.solid.addFace(WireId{}, plane), std::out_of_range);
}

TEST(TopologySolid, RejectsMissingRoot)
{
    Solid solid;
    EXPECT_FALSE(solid.isValid());
    EXPECT_THROW((void)solid.rootShell(), std::logic_error);
    EXPECT_THROW(solid.setRootShell(ShellId{}), std::out_of_range);
}

TEST(TopologyCurvedEdge, PreservesExactCircularGeometryInATwoEdgeWire)
{
    Solid solid;
    const auto right = solid.addVertex({2, 0, 0});
    const auto left = solid.addVertex({-2, 0, 0});
    const geometry::CircularArc3 upper{{0, 0, 0}, {0, 0, 1}, {1, 0, 0},
        2, 0, std::numbers::pi};
    const geometry::CircularArc3 lower{{0, 0, 0}, {0, 0, 1}, {1, 0, 0},
        2, std::numbers::pi, std::numbers::pi};

    const auto first = solid.addCircularEdge(right, left, upper);
    const auto second = solid.addCircularEdge(left, right, lower);
    const auto wire = solid.addWire({{first, EdgeOrientation::Forward},
                                     {second, EdgeOrientation::Forward}});

    ASSERT_EQ(solid.find(wire).uses().size(), 2u);
    ASSERT_TRUE(std::holds_alternative<geometry::CircularArc3>(solid.find(first).geometry()));
    const auto& stored = std::get<geometry::CircularArc3>(solid.find(first).geometry());
    EXPECT_DOUBLE_EQ(stored.radius(), 2.0);
    EXPECT_TRUE(geometry::areCoincident(stored.pointAt(0.5), geometry::Point3{0, 2, 0}));
    EXPECT_THROW((void)solid.segment(first), std::logic_error);
}

TEST(TopologyFace, SupportsInnerWiresAndCylindricalGeometry)
{
    Solid solid;
    const std::array outerVertices{solid.addVertex({-2, -2, 0}), solid.addVertex({2, -2, 0}),
        solid.addVertex({2, 2, 0}), solid.addVertex({-2, 2, 0})};
    const std::array outerEdges{solid.addEdge(outerVertices[0], outerVertices[1]),
        solid.addEdge(outerVertices[1], outerVertices[2]),
        solid.addEdge(outerVertices[2], outerVertices[3]),
        solid.addEdge(outerVertices[3], outerVertices[0])};
    const auto outer = solid.addWire({{outerEdges[0], EdgeOrientation::Forward},
        {outerEdges[1], EdgeOrientation::Forward}, {outerEdges[2], EdgeOrientation::Forward},
        {outerEdges[3], EdgeOrientation::Forward}});

    const auto right = solid.addVertex({1, 0, 0});
    const auto left = solid.addVertex({-1, 0, 0});
    const auto upper = solid.addCircularEdge(right, left,
        {{0, 0, 0}, {0, 0, 1}, {1, 0, 0}, 1, 0, std::numbers::pi});
    const auto lower = solid.addCircularEdge(left, right,
        {{0, 0, 0}, {0, 0, 1}, {1, 0, 0}, 1, std::numbers::pi, std::numbers::pi});
    const auto inner = solid.addWire({{lower, EdgeOrientation::Reversed},
                                      {upper, EdgeOrientation::Reversed}});

    const auto cap = solid.addFace(outer, {inner}, {{0, 0, 0}, {0, 0, 1}});
    EXPECT_EQ(solid.find(cap).innerWires(), std::vector<WireId>{inner});
    EXPECT_TRUE(std::holds_alternative<geometry::Plane>(solid.find(cap).support()));

    const auto cylinderFace = solid.addCylindricalFace(inner, {{0, 0, 0}, {0, 0, 1}, 1});
    ASSERT_TRUE(std::holds_alternative<geometry::Cylinder>(solid.find(cylinderFace).support()));
    EXPECT_DOUBLE_EQ(std::get<geometry::Cylinder>(solid.find(cylinderFace).support()).radius(), 1.0);
}

}
