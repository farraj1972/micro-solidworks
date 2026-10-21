#pragma once

#include "core/topology/Solid.h"
#include "features/FeatureId.h"
#include "interaction/TopologyTarget.h"

#include <array>
#include <vector>

namespace microsw::presentation
{
struct FaceTriangle
{
    std::array<geometry::Point3, 3> points;
    math::Vector3 normal;
    interaction::TopologyFaceTarget target;
};

struct PresentedEdge
{
    std::vector<geometry::Point3> points;
    interaction::TopologyEdgeTarget target;
    // Rebuild-local face targets incident to this visible CAD edge. Coplanar
    // seams are excluded with the edge, so face highlighting cannot invent one.
    std::vector<interaction::TopologyFaceTarget> incidentFaces;
};

struct SolidDisplayData
{
    std::vector<FaceTriangle> triangles;
    std::vector<PresentedEdge> edges;
    // Connected coplanar patches are one visual highlight region, while each
    // triangle still retains its exact rebuild-local FaceId for picking.
    std::vector<std::array<interaction::TopologyFaceTarget, 2>> coplanarNeighbors;
};

[[nodiscard]] SolidDisplayData deriveSolidDisplay(const topology::Solid& solid,
    features::FeatureId feature, interaction::BodyInstanceContext body,
    int curveSegments = 24);
[[nodiscard]] std::vector<interaction::TopologyFaceTarget> faceHighlightRegion(
    const SolidDisplayData& display, interaction::TopologyFaceTarget selected);
[[nodiscard]] std::vector<interaction::TopologyEdgeTarget> edgeHighlightRegion(
    const SolidDisplayData& display, interaction::TopologyEdgeTarget selected);
}
