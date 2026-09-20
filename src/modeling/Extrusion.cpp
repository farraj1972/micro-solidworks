#include "modeling/Extrusion.h"

#include "core/geometry/GeometricTolerance.h"

#include <cmath>
#include <stdexcept>

namespace microsw::modeling
{
topology::Solid extrude(const Profile& profile, math::Scalar distance)
{
    if (!std::isfinite(distance) || distance <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Extrusion distance must be finite and greater than geometric tolerance"};

    topology::Solid solid;
    const auto& points = profile.boundary();
    const auto count = points.size();
    const auto offset = profile.supportPlane().normal() * distance;
    std::vector<topology::VertexId> bottom, top;
    bottom.reserve(count); top.reserve(count);
    for (const auto& point : points) bottom.push_back(solid.addVertex(point));
    for (const auto& point : points) top.push_back(solid.addVertex(point + offset));

    std::vector<topology::EdgeId> bottomEdges, topEdges, verticalEdges;
    bottomEdges.reserve(count); topEdges.reserve(count); verticalEdges.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto next = (i + 1) % count;
        bottomEdges.push_back(solid.addEdge(bottom[i], bottom[next]));
        topEdges.push_back(solid.addEdge(top[i], top[next]));
        verticalEdges.push_back(solid.addEdge(bottom[i], top[i]));
    }

    std::vector<topology::OrientedEdgeUse> bottomUses, topUses;
    for (std::size_t i = 0; i < count; ++i)
    {
        bottomUses.push_back({bottomEdges[i], topology::EdgeOrientation::Forward});
        topUses.push_back({topEdges[i], topology::EdgeOrientation::Forward});
    }
    const auto bottomWire = solid.addWire(std::move(bottomUses));
    const auto topWire = solid.addWire(std::move(topUses));
    const auto bottomFace = solid.addFace(bottomWire, profile.supportPlane());
    const geometry::Plane topPlane{points.front() + offset, profile.supportPlane().normal()};
    const auto topFace = solid.addFace(topWire, topPlane);

    std::vector<topology::FaceId> sides;
    sides.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto next = (i + 1) % count;
        const auto wire = solid.addWire({
            {bottomEdges[i], topology::EdgeOrientation::Forward},
            {verticalEdges[next], topology::EdgeOrientation::Forward},
            {topEdges[i], topology::EdgeOrientation::Reversed},
            {verticalEdges[i], topology::EdgeOrientation::Reversed}});
        const auto outward = math::cross(
            geometry::Segment3{points[i], points[next]}.direction(), profile.supportPlane().normal());
        sides.push_back(solid.addFace(wire, {points[i], outward}));
    }

    std::vector<topology::OrientedFaceUse> faceUses{
        {bottomFace, topology::FaceOrientation::Reversed},
        {topFace, topology::FaceOrientation::Forward}};
    for (const auto face : sides)
        faceUses.push_back({face, topology::FaceOrientation::Forward});
    const auto shell = solid.addShell(std::move(faceUses));
    solid.setRootShell(shell);
    if (!solid.isValid()) throw std::runtime_error{"Extrusion produced an invalid Solid"};
    return solid;
}
}
