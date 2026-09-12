#include "core/topology/Solid.h"

#include "core/geometry/GeometricTolerance.h"
#include "core/math/Vector3.h"

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>

namespace microsw::topology
{
namespace
{

template<class Id, class Records>
const typename Records::value_type& checkedFind(Id id, const Records& records, const char* message)
{
    if (!id.isValid() || id.value() >= records.size())
        throw std::out_of_range{message};
    return records[id.value()];
}

EdgeOrientation flipped(EdgeOrientation orientation)
{
    return orientation == EdgeOrientation::Forward
        ? EdgeOrientation::Reversed : EdgeOrientation::Forward;
}

struct Traversal
{
    EdgeId edge;
    EdgeOrientation orientation;
};

std::vector<Traversal> shellBoundary(const Solid& solid, const OrientedFaceUse& faceUse)
{
    const auto& face = solid.find(faceUse.face);
    const auto& uses = solid.find(face.outerWire()).uses();
    std::vector<Traversal> result;
    result.reserve(uses.size());
    if (faceUse.orientation == FaceOrientation::Forward)
    {
        for (const auto& use : uses)
            result.push_back({use.edge, use.orientation});
    }
    else
    {
        for (auto it = uses.rbegin(); it != uses.rend(); ++it)
            result.push_back({it->edge, flipped(it->orientation)});
    }
    return result;
}

}

VertexId Solid::addVertex(const geometry::Point3& point)
{
    const auto id = VertexId{static_cast<VertexId::Value>(vertices_.size())};
    if (!id.isValid()) throw std::overflow_error{"Vertex ID space exhausted"};
    vertices_.emplace_back(point);
    return id;
}

EdgeId Solid::addEdge(VertexId start, VertexId end)
{
    const auto& first = find(start).point();
    const auto& second = find(end).point();
    if (start == end) throw std::invalid_argument{"Edge endpoints must be different VertexIds"};
    if (geometry::areCoincident(first, second))
        throw std::invalid_argument{"Edge endpoints must not be geometrically coincident"};
    const auto id = EdgeId{static_cast<EdgeId::Value>(edges_.size())};
    if (!id.isValid()) throw std::overflow_error{"Edge ID space exhausted"};
    edges_.emplace_back(start, end);
    return id;
}

VertexId Solid::traversalStart(const OrientedEdgeUse& use) const
{
    const auto& edge = find(use.edge);
    return use.orientation == EdgeOrientation::Forward ? edge.start() : edge.end();
}

VertexId Solid::traversalEnd(const OrientedEdgeUse& use) const
{
    const auto& edge = find(use.edge);
    return use.orientation == EdgeOrientation::Forward ? edge.end() : edge.start();
}

WireId Solid::addWire(std::vector<OrientedEdgeUse> uses)
{
    if (uses.size() < 3) throw std::invalid_argument{"A Wire requires at least three Edge uses"};
    std::set<std::uint32_t> usedEdges;
    for (std::size_t i = 0; i < uses.size(); ++i)
    {
        (void)find(uses[i].edge);
        if (!usedEdges.insert(uses[i].edge.value()).second)
            throw std::invalid_argument{"A Wire cannot repeat an EdgeId"};
        const auto next = (i + 1) % uses.size();
        if (traversalEnd(uses[i]) != traversalStart(uses[next]))
            throw std::invalid_argument{"Wire traversal is not closed by VertexId"};
    }
    const auto id = WireId{static_cast<WireId::Value>(wires_.size())};
    if (!id.isValid()) throw std::overflow_error{"Wire ID space exhausted"};
    wires_.emplace_back(std::move(uses));
    return id;
}

FaceId Solid::addFace(WireId outerWire, const geometry::Plane& supportPlane)
{
    const auto& wire = find(outerWire);
    std::vector<VertexId> vertices;
    vertices.reserve(wire.uses().size());
    std::set<std::uint32_t> uniqueVertices;
    for (const auto& use : wire.uses())
    {
        const auto vertex = traversalStart(use);
        vertices.push_back(vertex);
        uniqueVertices.insert(vertex.value());
        if (!supportPlane.contains(find(vertex).point()))
            throw std::invalid_argument{"Face boundary must be coplanar with its support Plane"};
    }
    if (uniqueVertices.size() < 3)
        throw std::invalid_argument{"A Face requires at least three distinct vertices"};

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const auto& a = find(vertices[i]).point();
        const auto& b = find(vertices[(i + 1) % vertices.size()]).point();
        const auto& c = find(vertices[(i + 2) % vertices.size()]).point();
        const auto incoming = geometry::Segment3{a, b}.direction();
        const auto outgoing = geometry::Segment3{b, c}.direction();
        const auto turn = math::dot(math::cross(incoming, outgoing), supportPlane.normal());
        if (turn <= geometry::defaultGeometricTolerance)
            throw std::invalid_argument{"Face boundary must be strictly convex and counter-clockwise"};
    }

    const auto id = FaceId{static_cast<FaceId::Value>(faces_.size())};
    if (!id.isValid()) throw std::overflow_error{"Face ID space exhausted"};
    faces_.emplace_back(outerWire, supportPlane);
    return id;
}

ShellId Solid::addShell(std::vector<OrientedFaceUse> uses)
{
    if (uses.empty()) throw std::invalid_argument{"A Shell requires Face uses"};
    std::set<std::uint32_t> uniqueFaces;
    std::map<std::uint32_t, std::vector<std::pair<std::size_t, EdgeOrientation>>> incidence;
    for (std::size_t faceIndex = 0; faceIndex < uses.size(); ++faceIndex)
    {
        (void)find(uses[faceIndex].face);
        if (!uniqueFaces.insert(uses[faceIndex].face.value()).second)
            throw std::invalid_argument{"A Shell cannot repeat a FaceId"};
        for (const auto& traversal : shellBoundary(*this, uses[faceIndex]))
            incidence[traversal.edge.value()].push_back({faceIndex, traversal.orientation});
    }

    std::vector<std::vector<std::size_t>> adjacency(uses.size());
    for (const auto& [edge, occurrences] : incidence)
    {
        (void)edge;
        if (occurrences.size() != 2)
            throw std::invalid_argument{"Every Shell Edge must be used exactly twice"};
        if (occurrences[0].second == occurrences[1].second)
            throw std::invalid_argument{"Incident Faces must traverse a shared Edge oppositely"};
        adjacency[occurrences[0].first].push_back(occurrences[1].first);
        adjacency[occurrences[1].first].push_back(occurrences[0].first);
    }

    std::vector<bool> visited(uses.size());
    std::queue<std::size_t> pending;
    pending.push(0);
    visited[0] = true;
    while (!pending.empty())
    {
        const auto current = pending.front(); pending.pop();
        for (const auto adjacent : adjacency[current])
            if (!visited[adjacent]) { visited[adjacent] = true; pending.push(adjacent); }
    }
    if (std::find(visited.begin(), visited.end(), false) != visited.end())
        throw std::invalid_argument{"Shell Faces must form one connected component"};

    const auto id = ShellId{static_cast<ShellId::Value>(shells_.size())};
    if (!id.isValid()) throw std::overflow_error{"Shell ID space exhausted"};
    shells_.emplace_back(std::move(uses));
    return id;
}

void Solid::setRootShell(ShellId shellId)
{
    const auto& shell = find(shellId);
    if (shells_.size() != 1)
        throw std::invalid_argument{"The B7 Solid supports exactly one Shell"};

    double cx = 0.0, cy = 0.0, cz = 0.0;
    for (std::size_t i = 0; i < vertices_.size(); ++i)
    {
        const auto factor = 1.0 / static_cast<double>(i + 1);
        cx += (vertices_[i].point().x() - cx) * factor;
        cy += (vertices_[i].point().y() - cy) * factor;
        cz += (vertices_[i].point().z() - cz) * factor;
    }
    const geometry::Point3 interior{cx, cy, cz};
    for (const auto& use : shell.uses())
    {
        const auto& plane = find(use.face).supportPlane();
        const auto normal = use.orientation == FaceOrientation::Forward
            ? plane.normal() : -plane.normal();
        const auto towardInterior = math::dot(interior - plane.origin(), normal);
        if (towardInterior >= -geometry::defaultGeometricTolerance)
            throw std::invalid_argument{"Shell Face uses must point outward from the convex Solid"};
    }
    rootShell_ = shellId;
}

geometry::Segment3 Solid::segment(EdgeId edgeId) const
{
    const auto& edge = find(edgeId);
    return {find(edge.start()).point(), find(edge.end()).point()};
}

const Vertex& Solid::find(VertexId id) const { return checkedFind(id, vertices_, "Invalid VertexId"); }
const Edge& Solid::find(EdgeId id) const { return checkedFind(id, edges_, "Invalid EdgeId"); }
const Wire& Solid::find(WireId id) const { return checkedFind(id, wires_, "Invalid WireId"); }
const Face& Solid::find(FaceId id) const { return checkedFind(id, faces_, "Invalid FaceId"); }
const Shell& Solid::find(ShellId id) const { return checkedFind(id, shells_, "Invalid ShellId"); }

const Shell& Solid::rootShell() const
{
    if (!rootShell_) throw std::logic_error{"Solid has no root Shell"};
    return find(*rootShell_);
}

bool Solid::isValid() const noexcept
{
    return rootShell_.has_value() && shells_.size() == 1;
}

}
