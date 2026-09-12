#include "core/topology/CuboidBuilder.h"

#include <array>
#include <cmath>
#include <stdexcept>

namespace microsw::topology
{

Solid makeCuboid(math::Scalar width, math::Scalar height, math::Scalar depth)
{
    if (!std::isfinite(width) || !std::isfinite(height) || !std::isfinite(depth)
        || width <= 0.0 || height <= 0.0 || depth <= 0.0)
        throw std::invalid_argument{"Cuboid dimensions must be finite and positive"};

    const auto x = width * 0.5;
    const auto y = height * 0.5;
    const auto z = depth * 0.5;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        throw std::overflow_error{"Cuboid coordinates are not finite"};

    Solid solid;
    const std::array<VertexId, 8> v{
        solid.addVertex({-x,-y,-z}), solid.addVertex({ x,-y,-z}),
        solid.addVertex({ x, y,-z}), solid.addVertex({-x, y,-z}),
        solid.addVertex({-x,-y, z}), solid.addVertex({ x,-y, z}),
        solid.addVertex({ x, y, z}), solid.addVertex({-x, y, z})};

    const std::array<std::pair<int,int>, 12> endpointIndices{{
        {0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}}};
    std::array<EdgeId, 12> edges;
    for (std::size_t i = 0; i < edges.size(); ++i)
        edges[i] = solid.addEdge(v[endpointIndices[i].first], v[endpointIndices[i].second]);

    const auto edgeUse = [&](int from, int to) {
        for (std::size_t i = 0; i < endpointIndices.size(); ++i)
        {
            const auto [a,b] = endpointIndices[i];
            if (a == from && b == to) return OrientedEdgeUse{edges[i], EdgeOrientation::Forward};
            if (a == to && b == from) return OrientedEdgeUse{edges[i], EdgeOrientation::Reversed};
        }
        throw std::logic_error{"Cuboid boundary references a missing Edge"};
    };
    const auto addBoundary = [&](std::array<int,4> cycle) {
        return solid.addWire({edgeUse(cycle[0],cycle[1]), edgeUse(cycle[1],cycle[2]),
                              edgeUse(cycle[2],cycle[3]), edgeUse(cycle[3],cycle[0])});
    };

    const std::array<WireId, 6> wires{
        addBoundary({0,1,2,3}), addBoundary({4,5,6,7}),
        addBoundary({0,1,5,4}), addBoundary({1,2,6,5}),
        addBoundary({3,7,6,2}), addBoundary({0,4,7,3})};
    const std::array<FaceId, 6> faces{
        solid.addFace(wires[0], {{0,0,-z},{0,0,1}}),
        solid.addFace(wires[1], {{0,0, z},{0,0, 1}}),
        solid.addFace(wires[2], {{0,-y,0},{0,-1,0}}),
        solid.addFace(wires[3], {{ x,0,0},{1,0,0}}),
        solid.addFace(wires[4], {{0, y,0},{0,1,0}}),
        solid.addFace(wires[5], {{-x,0,0},{-1,0,0}})};
    std::vector<OrientedFaceUse> faceUses;
    faceUses.push_back({faces[0], FaceOrientation::Reversed});
    for (std::size_t i = 1; i < faces.size(); ++i)
        faceUses.push_back({faces[i], FaceOrientation::Forward});
    const auto shell = solid.addShell(std::move(faceUses));
    solid.setRootShell(shell);
    return solid;
}

}
