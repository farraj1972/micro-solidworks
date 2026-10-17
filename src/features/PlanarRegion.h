#pragma once

#include "core/geometry/Arc2.h"
#include "core/geometry/Circle2.h"
#include "core/geometry/Segment2.h"
#include "core/sketch/Sketch.h"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace microsw::features
{
using RegionCurve2 = std::variant<geometry::Segment2, geometry::Arc2, geometry::Circle2>;
struct RegionBoundaryUse { sketch::SketchEntityId source; RegionCurve2 curve; bool reversed{}; };
struct RegionBoundary { std::vector<RegionBoundaryUse> uses; };
struct PlanarRegion { RegionBoundary outer; std::vector<RegionBoundary> holes; };

enum class RegionStatus { Success, Empty, OpenContour, Branch, SelfIntersection, Overlap,
    Degenerate, Ambiguous, UnsupportedGeometry };
struct RegionDiagnostic
{
    RegionStatus status{RegionStatus::Success};
    std::string message;
    std::vector<sketch::SketchEntityId> entities;
};
struct PlanarRegionSet
{
    std::vector<PlanarRegion> regions;
    RegionDiagnostic diagnostic;
    [[nodiscard]] bool valid() const noexcept { return diagnostic.status == RegionStatus::Success && !regions.empty(); }
};

[[nodiscard]] PlanarRegionSet extractPlanarRegions(const sketch::Sketch& sketch);
}
