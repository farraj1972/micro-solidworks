#pragma once

#include "features/FeatureModel.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace microsw::exporting
{
struct MeshQuality
{
    double chordalDeviationMm{0.02};
    double angularDeviationRadians{0.1};
    double weldToleranceMm{1e-8};
    std::size_t maxTriangles{200000};
};

struct MeshBounds
{
    geometry::Point3 minimum;
    geometry::Point3 maximum;
};

struct ExportMesh
{
    std::size_t sourceBodyIndex{};
    std::vector<geometry::Point3> vertices;
    std::vector<std::array<std::uint32_t, 3>> triangles;
    MeshBounds bounds;
    double volumeMm3{};
};

enum class MeshStatus { Valid, InvalidInput, Unsupported, InvalidMesh, ResourceLimit };
struct MeshResult
{
    MeshStatus status{MeshStatus::InvalidInput};
    std::string diagnostic;
    std::optional<ExportMesh> mesh;
    std::vector<std::string> warnings;
    [[nodiscard]] bool succeeded() const noexcept { return status == MeshStatus::Valid; }
};

// The selected current FeatureModel result is the only CAD source. bodyIndex
// identifies one body of that result; historical feature caches are never read.
[[nodiscard]] MeshResult buildExportMesh(const features::FeatureModel& model,
    std::size_t bodyIndex, const MeshQuality& quality = {});
[[nodiscard]] MeshResult buildExportMesh(const topology::Solid& solid,
    const MeshQuality& quality = {});

// Also validates test/consumer supplied meshes without trusting their metadata.
[[nodiscard]] MeshResult validateExportMesh(ExportMesh mesh,
    const MeshQuality& quality = {});
}
