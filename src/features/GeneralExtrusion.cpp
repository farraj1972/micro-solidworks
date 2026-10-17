#include "features/GeneralExtrusion.h"

#include "core/geometry/GeometricTolerance.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace microsw::features
{
namespace
{
struct CurvePiece
{
    geometry::Point2 start;
    geometry::Point2 end;
    std::variant<geometry::Segment2, geometry::Arc2> geometry;
};

std::vector<CurvePiece> pieces(const RegionBoundary& boundary)
{
    std::vector<CurvePiece> result;
    for (const auto& use : boundary.uses)
    {
        std::visit([&](const auto& curve) {
            using T = std::decay_t<decltype(curve)>;
            if constexpr (std::is_same_v<T, geometry::Segment2>)
            {
                result.push_back(use.reversed
                    ? CurvePiece{curve.b(), curve.a(), geometry::Segment2{curve.b(), curve.a()}}
                    : CurvePiece{curve.a(), curve.b(), curve});
            }
            else if constexpr (std::is_same_v<T, geometry::Arc2>)
            {
                const auto start = use.reversed ? curve.endPoint() : curve.startPoint();
                const auto end = use.reversed ? curve.startPoint() : curve.endPoint();
                const auto angle = use.reversed
                    ? curve.startAngle() + curve.sweepAngle() : curve.startAngle();
                const auto sweep = use.reversed ? -curve.sweepAngle() : curve.sweepAngle();
                result.push_back({start, end,
                    geometry::Arc2{curve.center(), curve.radius(), angle, sweep}});
            }
            else
            {
                const auto sweep = use.reversed
                    ? -std::numbers::pi : std::numbers::pi;
                const auto firstStart = 0.0;
                const auto secondStart = sweep;
                const geometry::Arc2 first{curve.center(), curve.radius(), firstStart, sweep};
                const geometry::Arc2 second{curve.center(), curve.radius(), secondStart, sweep};
                result.push_back({first.startPoint(), first.endPoint(), first});
                result.push_back({second.startPoint(), second.endPoint(), second});
            }
        }, use.curve);
    }
    return result;
}

struct BuiltBoundary
{
    topology::WireId bottomWire;
    topology::WireId topWire;
    std::vector<topology::FaceId> sideFaces;
};

topology::EdgeId addCurveEdge(topology::Solid& solid,
                              topology::VertexId start,
                              topology::VertexId end,
                              const CurvePiece& piece,
                              const sketch::SketchPlane& plane,
                              const math::Vector3& offset)
{
    return std::visit([&](const auto& curve) -> topology::EdgeId {
        using T = std::decay_t<decltype(curve)>;
        if constexpr (std::is_same_v<T, geometry::Segment2>)
        {
            return solid.addEdge(start, end);
        }
        else
        {
            const geometry::CircularArc3 arc{
                plane.toWorld(curve.center()) + offset,
                plane.normal(), plane.xAxis(), curve.radius(),
                curve.startAngle(), curve.sweepAngle()};
            return solid.addCircularEdge(start, end, arc);
        }
    }, piece.geometry);
}

BuiltBoundary buildBoundary(topology::Solid& solid,
                            const RegionBoundary& boundary,
                            const sketch::SketchPlane& plane,
                            math::Scalar distance)
{
    const auto curvePieces = pieces(boundary);
    if (curvePieces.size() < 2)
        throw std::invalid_argument{"A region boundary requires at least two edge pieces"};

    const auto offset = plane.normal() * distance;
    std::vector<topology::VertexId> bottomVertices, topVertices;
    bottomVertices.reserve(curvePieces.size());
    topVertices.reserve(curvePieces.size());
    for (const auto& piece : curvePieces)
    {
        const auto point = plane.toWorld(piece.start);
        bottomVertices.push_back(solid.addVertex(point));
        topVertices.push_back(solid.addVertex(point + offset));
    }

    std::vector<topology::EdgeId> bottomEdges, topEdges, verticalEdges;
    bottomEdges.reserve(curvePieces.size());
    topEdges.reserve(curvePieces.size());
    verticalEdges.reserve(curvePieces.size());
    for (std::size_t index = 0; index < curvePieces.size(); ++index)
    {
        const auto next = (index + 1) % curvePieces.size();
        bottomEdges.push_back(addCurveEdge(solid, bottomVertices[index],
            bottomVertices[next], curvePieces[index], plane, {}));
        topEdges.push_back(addCurveEdge(solid, topVertices[index],
            topVertices[next], curvePieces[index], plane, offset));
        verticalEdges.push_back(solid.addEdge(bottomVertices[index], topVertices[index]));
    }

    std::vector<topology::OrientedEdgeUse> bottomUses, topUses;
    for (std::size_t index = 0; index < curvePieces.size(); ++index)
    {
        bottomUses.push_back({bottomEdges[index], topology::EdgeOrientation::Forward});
        topUses.push_back({topEdges[index], topology::EdgeOrientation::Forward});
    }
    BuiltBoundary result{solid.addWire(std::move(bottomUses)),
                         solid.addWire(std::move(topUses)), {}};

    for (std::size_t index = 0; index < curvePieces.size(); ++index)
    {
        const auto next = (index + 1) % curvePieces.size();
        const auto sideWire = solid.addWire({
            {bottomEdges[index], topology::EdgeOrientation::Forward},
            {verticalEdges[next], topology::EdgeOrientation::Forward},
            {topEdges[index], topology::EdgeOrientation::Reversed},
            {verticalEdges[index], topology::EdgeOrientation::Reversed}});
        const auto side = std::visit([&](const auto& curve) -> topology::FaceId {
            using T = std::decay_t<decltype(curve)>;
            if constexpr (std::is_same_v<T, geometry::Segment2>)
            {
                const auto tangent = plane.toWorld(curve.b()) - plane.toWorld(curve.a());
                return solid.addFace(sideWire,
                    {plane.toWorld(curve.a()), math::cross(tangent, plane.normal())});
            }
            else
            {
                return solid.addCylindricalFace(sideWire,
                    {plane.toWorld(curve.center()), plane.normal(), curve.radius()});
            }
        }, curvePieces[index].geometry);
        result.sideFaces.push_back(side);
    }
    return result;
}

topology::Solid extrudeRegion(const PlanarRegion& region,
                              const sketch::SketchPlane& plane,
                              math::Scalar distance)
{
    topology::Solid solid;
    auto outer = buildBoundary(solid, region.outer, plane, distance);
    std::vector<BuiltBoundary> holes;
    holes.reserve(region.holes.size());
    for (const auto& hole : region.holes)
        holes.push_back(buildBoundary(solid, hole, plane, distance));

    std::vector<topology::WireId> bottomHoles, topHoles;
    for (const auto& hole : holes)
    {
        bottomHoles.push_back(hole.bottomWire);
        topHoles.push_back(hole.topWire);
    }
    const auto bottomFace = solid.addFace(outer.bottomWire, std::move(bottomHoles), plane.plane());
    const geometry::Plane topPlane{plane.origin() + plane.normal() * distance, plane.normal()};
    const auto topFace = solid.addFace(outer.topWire, std::move(topHoles), topPlane);

    std::vector<topology::OrientedFaceUse> faces{
        {bottomFace, topology::FaceOrientation::Reversed},
        {topFace, topology::FaceOrientation::Forward}};
    for (const auto face : outer.sideFaces)
        faces.push_back({face, topology::FaceOrientation::Forward});
    for (const auto& hole : holes)
        for (const auto face : hole.sideFaces)
            faces.push_back({face, topology::FaceOrientation::Forward});

    const auto shell = solid.addShell(std::move(faces));
    solid.setGeneralRootShell(shell);
    if (!solid.isValid())
        throw std::runtime_error{"General extrusion produced an invalid Solid"};
    return solid;
}
}

GeneralExtrusionResult extrudeRegions(const PlanarRegionSet& regions,
                                      const sketch::SketchPlane& plane,
                                      math::Scalar distance)
{
    if (!regions.valid())
        return {GeneralExtrusionStatus::InvalidRegions, {}, regions.diagnostic.message};
    if (!std::isfinite(distance) || distance <= geometry::defaultGeometricTolerance)
        return {GeneralExtrusionStatus::InvalidDistance, {},
            "Extrusion distance must be finite and greater than geometric tolerance"};

    try
    {
        std::vector<topology::Solid> candidates;
        candidates.reserve(regions.regions.size());
        for (const auto& region : regions.regions)
            candidates.push_back(extrudeRegion(region, plane, distance));
        return {GeneralExtrusionStatus::Success, std::move(candidates), {}};
    }
    catch (const std::exception& error)
    {
        return {GeneralExtrusionStatus::TopologyFailure, {}, error.what()};
    }
}
}
