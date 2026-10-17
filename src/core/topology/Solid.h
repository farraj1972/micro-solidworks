#pragma once

#include "core/geometry/Plane.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/CircularArc3.h"
#include "core/geometry/Cylinder.h"
#include "core/topology/TopologyIds.h"

#include <optional>
#include <utility>
#include <vector>
#include <variant>

namespace microsw::topology
{

enum class EdgeOrientation { Forward, Reversed };
enum class FaceOrientation { Forward, Reversed };

struct OrientedEdgeUse
{
    EdgeId edge;
    EdgeOrientation orientation{EdgeOrientation::Forward};
    friend bool operator==(const OrientedEdgeUse&, const OrientedEdgeUse&) = default;
};

struct OrientedFaceUse
{
    FaceId face;
    FaceOrientation orientation{FaceOrientation::Forward};
    friend bool operator==(const OrientedFaceUse&, const OrientedFaceUse&) = default;
};

class Vertex
{
public:
    explicit Vertex(const geometry::Point3& point) : point_{point} {}
    [[nodiscard]] const geometry::Point3& point() const noexcept { return point_; }
private:
    geometry::Point3 point_;
};

class Edge
{
public:
    using Geometry = std::variant<geometry::Segment3, geometry::CircularArc3>;
    Edge(VertexId start, VertexId end, Geometry geometry) : start_{start}, end_{end}, geometry_{std::move(geometry)} {}
    [[nodiscard]] VertexId start() const noexcept { return start_; }
    [[nodiscard]] VertexId end() const noexcept { return end_; }
    [[nodiscard]] const Geometry& geometry() const noexcept { return geometry_; }
private:
    VertexId start_;
    VertexId end_;
    Geometry geometry_;
};

class Wire
{
public:
    explicit Wire(std::vector<OrientedEdgeUse> uses) : uses_{std::move(uses)} {}
    [[nodiscard]] const std::vector<OrientedEdgeUse>& uses() const noexcept { return uses_; }
private:
    std::vector<OrientedEdgeUse> uses_;
};

class Face
{
public:
    using Support = std::variant<geometry::Plane, geometry::Cylinder>;
    Face(WireId outerWire, std::vector<WireId> innerWires, Support support)
        : outerWire_{outerWire}, innerWires_{std::move(innerWires)}, support_{std::move(support)} {}
    [[nodiscard]] WireId outerWire() const noexcept { return outerWire_; }
    [[nodiscard]] const std::vector<WireId>& innerWires() const noexcept { return innerWires_; }
    [[nodiscard]] const Support& support() const noexcept { return support_; }
    [[nodiscard]] const geometry::Plane& supportPlane() const { return std::get<geometry::Plane>(support_); }
private:
    WireId outerWire_;
    std::vector<WireId> innerWires_;
    Support support_;
};

class Shell
{
public:
    explicit Shell(std::vector<OrientedFaceUse> uses) : uses_{std::move(uses)} {}
    [[nodiscard]] const std::vector<OrientedFaceUse>& uses() const noexcept { return uses_; }
private:
    std::vector<OrientedFaceUse> uses_;
};

class Solid
{
public:
    VertexId addVertex(const geometry::Point3& point);
    EdgeId addEdge(VertexId start, VertexId end);
    EdgeId addCircularEdge(VertexId start, VertexId end, const geometry::CircularArc3& arc);
    WireId addWire(std::vector<OrientedEdgeUse> uses);
    FaceId addFace(WireId outerWire, const geometry::Plane& supportPlane);
    FaceId addFace(WireId outerWire, std::vector<WireId> innerWires, const geometry::Plane& supportPlane);
    FaceId addCylindricalFace(WireId outerWire, const geometry::Cylinder& support);
    ShellId addShell(std::vector<OrientedFaceUse> uses);
    void setRootShell(ShellId shell);
    void setCellDerivedRootShell(ShellId shell);
    void setGeneralRootShell(ShellId shell);

    [[nodiscard]] geometry::Segment3 segment(EdgeId edge) const;
    [[nodiscard]] const std::vector<Vertex>& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const std::vector<Edge>& edges() const noexcept { return edges_; }
    [[nodiscard]] const std::vector<Wire>& wires() const noexcept { return wires_; }
    [[nodiscard]] const std::vector<Face>& faces() const noexcept { return faces_; }
    [[nodiscard]] const std::vector<Shell>& shells() const noexcept { return shells_; }

    [[nodiscard]] const Vertex& find(VertexId id) const;
    [[nodiscard]] const Edge& find(EdgeId id) const;
    [[nodiscard]] const Wire& find(WireId id) const;
    [[nodiscard]] const Face& find(FaceId id) const;
    [[nodiscard]] const Shell& find(ShellId id) const;
    [[nodiscard]] std::optional<ShellId> rootShellId() const noexcept { return rootShell_; }
    [[nodiscard]] const Shell& rootShell() const;
    [[nodiscard]] bool isValid() const noexcept;

private:
    [[nodiscard]] VertexId traversalStart(const OrientedEdgeUse& use) const;
    [[nodiscard]] VertexId traversalEnd(const OrientedEdgeUse& use) const;

    std::vector<Vertex> vertices_;
    std::vector<Edge> edges_;
    std::vector<Wire> wires_;
    std::vector<Face> faces_;
    std::vector<Shell> shells_;
    std::optional<ShellId> rootShell_;
};

}
