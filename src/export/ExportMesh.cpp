#include "export/ExportMesh.h"

#include "presentation/SolidDisplay.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <numbers>
#include <queue>
#include <tuple>
#include <variant>

namespace microsw::exporting
{
namespace
{
using Triangle = std::array<std::uint32_t, 3>;
using Edge = std::pair<std::uint32_t, std::uint32_t>;
using QuantizedPoint = std::tuple<std::int64_t, std::int64_t, std::int64_t>;
struct EdgeUse { std::size_t triangle; bool forward; };

MeshResult fail(MeshStatus status, std::string diagnostic)
{
    return {status, std::move(diagnostic), std::nullopt};
}

bool validQuality(const MeshQuality& quality)
{
    return std::isfinite(quality.chordalDeviationMm) && quality.chordalDeviationMm > 0
        && std::isfinite(quality.angularDeviationRadians) && quality.angularDeviationRadians > 0
        && quality.angularDeviationRadians < std::numbers::pi
        && std::isfinite(quality.weldToleranceMm) && quality.weldToleranceMm > 0
        && quality.maxTriangles > 0;
}

bool finite(const geometry::Point3& p)
{
    return std::isfinite(p.x()) && std::isfinite(p.y()) && std::isfinite(p.z());
}

std::optional<QuantizedPoint> quantize(const geometry::Point3& p, double tolerance)
{
    if (!finite(p)) return std::nullopt;
    const auto withinRange = [tolerance](double value) {
        return std::abs(value / tolerance) < static_cast<double>(std::numeric_limits<std::int64_t>::max()) / 4;
    };
    if (!withinRange(p.x()) || !withinRange(p.y()) || !withinRange(p.z())) return std::nullopt;
    return QuantizedPoint{std::llround(p.x() / tolerance),
        std::llround(p.y() / tolerance), std::llround(p.z() / tolerance)};
}

int curveSegments(const topology::Solid& solid, const MeshQuality& quality)
{
    int count = 3;
    for (const auto& edge : solid.edges())
    {
        const auto* arc = std::get_if<geometry::CircularArc3>(&edge.geometry());
        if (!arc) continue;
        const double sweep = std::abs(arc->sweepAngle());
        const double ratio = std::clamp(1.0 - quality.chordalDeviationMm / arc->radius(), -1.0, 1.0);
        const double chordAngle = 2.0 * std::acos(ratio);
        const double step = std::min(quality.angularDeviationRadians, chordAngle);
        if (!(step > 0) || !std::isfinite(step)) return -1;
        const double required = std::ceil(sweep / step);
        if (required > 4096) return -1;
        count = std::max(count, static_cast<int>(required));
    }
    return count;
}

double triangleVolume(const geometry::Point3& a, const geometry::Point3& b,
    const geometry::Point3& c)
{
    return math::dot(a - geometry::Point3{0,0,0}, math::cross(b - a, c - a)) / 6.0;
}
}

static MeshResult validateMesh(ExportMesh mesh, const MeshQuality& quality, bool reconcileRawWinding)
{
    if (!validQuality(quality)) return fail(MeshStatus::InvalidInput, "Invalid mesh quality settings");
    if (mesh.triangles.empty() || mesh.vertices.empty())
        return fail(MeshStatus::InvalidMesh, "Export mesh is empty");
    if (mesh.triangles.size() > quality.maxTriangles)
        return fail(MeshStatus::ResourceLimit, "Export triangle limit exceeded");
    for (const auto& point : mesh.vertices)
        if (!finite(point)) return fail(MeshStatus::InvalidMesh, "Nonfinite mesh coordinate");

    std::map<Edge, std::vector<EdgeUse>> edges;
    for (std::size_t i = 0; i < mesh.triangles.size(); ++i)
    {
        const auto& t = mesh.triangles[i];
        if (t[0] >= mesh.vertices.size() || t[1] >= mesh.vertices.size()
            || t[2] >= mesh.vertices.size())
            return fail(MeshStatus::InvalidMesh, "Triangle index outside vertex array");
        if (t[0] == t[1] || t[1] == t[2] || t[2] == t[0])
            return fail(MeshStatus::InvalidMesh, "Degenerate triangle indices");
        const auto& a = mesh.vertices[t[0]];
        const auto& b = mesh.vertices[t[1]];
        const auto& c = mesh.vertices[t[2]];
        const double doubledArea = math::cross(b-a, c-a).length();
        if (!std::isfinite(doubledArea) || doubledArea <= quality.weldToleranceMm * quality.weldToleranceMm)
            return fail(MeshStatus::InvalidMesh, "Degenerate or nonfinite triangle");
        for (int j = 0; j < 3; ++j)
        {
            const auto from = t[j], to = t[(j+1)%3];
            const Edge key{std::min(from,to), std::max(from,to)};
            edges[key].push_back({i, from < to});
        }
    }

    std::vector<std::vector<std::pair<std::size_t, bool>>> neighbors(mesh.triangles.size());
    for (const auto& [edge, uses] : edges)
    {
        if (uses.size() != 2)
            return fail(MeshStatus::InvalidMesh, "Open or nonmanifold mesh edge");
        if (uses[0].triangle == uses[1].triangle)
            return fail(MeshStatus::InvalidMesh, "Self-incident mesh edge");
        const bool sameDirection = uses[0].forward == uses[1].forward;
        if (sameDirection && !reconcileRawWinding)
            return fail(MeshStatus::InvalidMesh, "Inconsistent triangle winding");
        neighbors[uses[0].triangle].push_back({uses[1].triangle, sameDirection});
        neighbors[uses[1].triangle].push_back({uses[0].triangle, sameDirection});
    }

    // Reconcile raw per-face tessellation winding over the connected triangle
    // graph. A conflicting cycle is non-orientable and must not be exported.
    std::vector<int> flip(mesh.triangles.size(), -1);
    std::vector<std::vector<std::size_t>> components;
    for (std::size_t start = 0; start < mesh.triangles.size(); ++start)
    {
        if (flip[start] != -1) continue;
        components.emplace_back();
        std::queue<std::size_t> pending;
        pending.push(start); flip[start] = 0;
        while (!pending.empty())
        {
            const auto current = pending.front(); pending.pop();
            components.back().push_back(current);
            for (const auto [other, sameDirection] : neighbors[current])
            {
                const int needed = flip[current] ^ static_cast<int>(sameDirection);
                if (flip[other] != -1 && flip[other] != needed)
                    return fail(MeshStatus::InvalidMesh, "Inconsistent mesh winding cycle");
                if (flip[other] == -1) { flip[other] = needed; pending.push(other); }
            }
        }
    }
    if (components.size() != 1)
        return fail(MeshStatus::Unsupported, "Select one connected CAD body per export mesh");
    for (std::size_t i = 0; i < mesh.triangles.size(); ++i)
        if (flip[i]) std::swap(mesh.triangles[i][1], mesh.triangles[i][2]);

    mesh.volumeMm3 = 0;
    for (const auto& component : components)
    {
        double signedVolume = 0;
        for (const auto index : component)
        {
            const auto& t = mesh.triangles[index];
            signedVolume += triangleVolume(mesh.vertices[t[0]], mesh.vertices[t[1]], mesh.vertices[t[2]]);
        }
        if (!std::isfinite(signedVolume) || std::abs(signedVolume) <= quality.weldToleranceMm)
            return fail(MeshStatus::InvalidMesh, "Mesh component has no enclosed volume");
        if (signedVolume < 0 && !reconcileRawWinding)
            return fail(MeshStatus::InvalidMesh, "Inward mesh component orientation");
        if (signedVolume < 0)
        {
            for (const auto index : component)
                std::swap(mesh.triangles[index][1], mesh.triangles[index][2]);
            signedVolume = -signedVolume;
        }
        mesh.volumeMm3 += signedVolume;
    }

    double minX = mesh.vertices[0].x(), minY = mesh.vertices[0].y(), minZ = mesh.vertices[0].z();
    double maxX = minX, maxY = minY, maxZ = minZ;
    for (const auto& p : mesh.vertices)
    {
        minX=std::min(minX,p.x()); minY=std::min(minY,p.y()); minZ=std::min(minZ,p.z());
        maxX=std::max(maxX,p.x()); maxY=std::max(maxY,p.y()); maxZ=std::max(maxZ,p.z());
    }
    mesh.bounds = {{minX,minY,minZ},{maxX,maxY,maxZ}};
    return {MeshStatus::Valid, {}, std::move(mesh)};
}

MeshResult validateExportMesh(ExportMesh mesh, const MeshQuality& quality)
{
    return validateMesh(std::move(mesh), quality, false);
}

MeshResult buildExportMesh(const topology::Solid& solid, const MeshQuality& quality)
{
    if (!validQuality(quality)) return fail(MeshStatus::InvalidInput, "Invalid mesh quality settings");
    if (!solid.isValid()) return fail(MeshStatus::InvalidInput, "Current CAD Solid is invalid");
    const int segments = curveSegments(solid, quality);
    if (segments < 0) return fail(MeshStatus::ResourceLimit, "Curve subdivision limit exceeded");
    const auto display = presentation::deriveSolidDisplay(solid, features::FeatureId{0}, {0}, segments);
    if (display.triangles.size() > quality.maxTriangles)
        return fail(MeshStatus::ResourceLimit, "Export triangle limit exceeded");

    ExportMesh mesh;
    std::map<QuantizedPoint, std::uint32_t> index;
    for (const auto& triangle : display.triangles)
    {
        Triangle t{};
        for (int corner=0; corner<3; ++corner)
        {
            const auto& point = triangle.points[corner];
            const auto key = quantize(point, quality.weldToleranceMm);
            if (!key) return fail(MeshStatus::InvalidMesh, "Nonfinite or unrepresentable CAD coordinate");
            const auto [it, inserted] = index.try_emplace(*key, static_cast<std::uint32_t>(mesh.vertices.size()));
            if (inserted) mesh.vertices.push_back(point);
            t[corner] = it->second;
        }
        mesh.triangles.push_back(t);
    }
    return validateMesh(std::move(mesh), quality, true);
}

MeshResult buildExportMesh(const features::FeatureModel& model,
    std::size_t bodyIndex, const MeshQuality& quality)
{
    const auto resultId = model.resultFeatureId();
    if (!resultId.isValid()) return fail(MeshStatus::InvalidInput, "No current CAD result");
    const auto& result = model.find(resultId);
    if (result.state() != features::FeatureState::Clean || !result.cache())
        return fail(MeshStatus::InvalidInput, "Current CAD result is not rebuilt successfully");
    const auto* solidCache = std::get_if<features::SolidFeatureCache>(&*result.cache());
    if (!solidCache || bodyIndex >= solidCache->solids.size())
        return fail(MeshStatus::InvalidInput, "Select a body of the current CAD result");
    auto mesh = buildExportMesh(solidCache->solids[bodyIndex], quality);
    if (mesh.mesh) mesh.mesh->sourceBodyIndex = bodyIndex;
    return mesh;
}
}
