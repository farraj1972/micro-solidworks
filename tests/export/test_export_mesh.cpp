#include "export/ExportMesh.h"
#include "core/topology/CuboidBuilder.h"
#include "features/GeneralExtrusion.h"
#include "features/PlanarRegion.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <utility>

namespace
{
using namespace microsw;

features::FeatureModel makePlate(double halfWidth = 40, double depth = 2)
{
    features::FeatureModel model;
    sketch::Sketch base;
    (void)base.addRectangle({-halfWidth,-25},{halfWidth,25});
    (void)base.addCircle({{-22,0},4});
    const auto extrude = model.addExtrude(model.addSketch(std::move(base)),8);
    sketch::Sketch pocket;
    (void)pocket.addRectangle({7,-7},{25,7});
    const auto pocketId = model.addFaceSketch(std::move(pocket), extrude,
        features::ExtrudeCapRole::EndCap);
    (void)model.addCutExtrude(pocketId, extrude, depth, features::CutDirection::Reverse);
    if (model.rebuild().status != features::RebuildStatus::Success)
        throw std::runtime_error("Reference plate rebuild failed");
    return model;
}

TEST(ExportMesh, ReferencePlateIsClosedAndMatchesAnalyticOracles)
{
    for (const auto [halfWidth,depth] : {std::pair{40.0,2.0},
        std::pair{45.0,2.0},std::pair{45.0,3.0}})
    {
        const auto model = makePlate(halfWidth,depth);
        const auto result = exporting::buildExportMesh(model,0);
        ASSERT_TRUE(result.succeeded()) << result.diagnostic;
        const auto& mesh = *result.mesh;
        const double expected = 2*halfWidth*50*8 - std::numbers::pi*4*4*8 - 18*14*depth;
        EXPECT_NEAR(mesh.volumeMm3, expected, 1.0);
        EXPECT_NEAR(mesh.bounds.minimum.x(),-halfWidth,1e-8);
        EXPECT_NEAR(mesh.bounds.maximum.x(),halfWidth,1e-8);
        EXPECT_NEAR(mesh.bounds.minimum.y(),-25,1e-8);
        EXPECT_NEAR(mesh.bounds.maximum.y(),25,1e-8);
        EXPECT_NEAR(mesh.bounds.minimum.z(),0,1e-8);
        EXPECT_NEAR(mesh.bounds.maximum.z(),8,1e-8);
        EXPECT_EQ(exporting::buildExportMesh(model,0).mesh->triangles,mesh.triangles);
    }
}

TEST(ExportMesh, RejectsInvalidSourceAndCorruptMeshes)
{
    features::FeatureModel empty;
    EXPECT_EQ(exporting::buildExportMesh(empty,0).status,exporting::MeshStatus::InvalidInput);
    const auto model = makePlate();
    EXPECT_EQ(exporting::buildExportMesh(model,1).status,exporting::MeshStatus::InvalidInput);
    const auto good = exporting::buildExportMesh(model,0);
    ASSERT_TRUE(good.succeeded()) << good.diagnostic;
    auto broken = *good.mesh;
    broken.triangles.pop_back();
    EXPECT_EQ(exporting::validateExportMesh(std::move(broken)).status,exporting::MeshStatus::InvalidMesh);
    broken = *good.mesh;
    broken.triangles[0][1] = broken.triangles[0][0];
    EXPECT_EQ(exporting::validateExportMesh(std::move(broken)).status,exporting::MeshStatus::InvalidMesh);
    broken = *good.mesh;
    std::swap(broken.triangles[0][1],broken.triangles[0][2]);
    EXPECT_EQ(exporting::validateExportMesh(std::move(broken)).status,exporting::MeshStatus::InvalidMesh);
    EXPECT_THROW((geometry::Point3{std::numeric_limits<double>::quiet_NaN(),0,0}),
        std::invalid_argument);
    exporting::MeshQuality bounded;
    bounded.maxTriangles = 1;
    EXPECT_EQ(exporting::buildExportMesh(model,0,bounded).status,exporting::MeshStatus::ResourceLimit);
}

TEST(ExportMesh, BoxCylinderAndHoledPlateHaveValidatedOutwardMeshes)
{
    const auto box = exporting::buildExportMesh(topology::makeCuboid(2,3,4));
    ASSERT_TRUE(box.succeeded()) << box.diagnostic;
    EXPECT_NEAR(box.mesh->volumeMm3,24,1e-9);

    sketch::Sketch disk;
    (void)disk.addCircle({{0,0},4});
    const auto cylinder = features::extrudeRegions(
        features::extractPlanarRegions(disk),disk.plane(),8);
    ASSERT_TRUE(cylinder.succeeded());
    const auto roundMesh = exporting::buildExportMesh(cylinder.solids.front());
    ASSERT_TRUE(roundMesh.succeeded()) << roundMesh.diagnostic;
    EXPECT_NEAR(roundMesh.mesh->volumeMm3,std::numbers::pi*16*8,1.0);

    sketch::Sketch ring;
    (void)ring.addRectangle({-40,-25},{40,25});
    (void)ring.addCircle({{-22,0},4});
    const auto plate = features::extrudeRegions(
        features::extractPlanarRegions(ring),ring.plane(),8);
    ASSERT_TRUE(plate.succeeded());
    const auto plateMesh = exporting::buildExportMesh(plate.solids.front());
    ASSERT_TRUE(plateMesh.succeeded()) << plateMesh.diagnostic;
    EXPECT_NEAR(plateMesh.mesh->volumeMm3,32000-std::numbers::pi*16*8,1.0);
}

TEST(ExportMesh, CurrentResultAndSingleConnectedBodyPolicy)
{
    features::FeatureModel model;
    sketch::Sketch first;
    (void)first.addRectangle({0,0},{2,3});
    const auto extrude = model.addExtrude(model.addSketch(std::move(first)),4);
    EXPECT_EQ(exporting::buildExportMesh(model,0).status,exporting::MeshStatus::InvalidInput);
    ASSERT_EQ(model.rebuild().status,features::RebuildStatus::Success);
    const auto valid = exporting::buildExportMesh(model,0);
    ASSERT_TRUE(valid.succeeded()) << valid.diagnostic;
    EXPECT_NEAR(valid.mesh->volumeMm3,24,1e-9);

    auto disconnected = *valid.mesh;
    const auto offset = static_cast<std::uint32_t>(disconnected.vertices.size());
    for (const auto& point : valid.mesh->vertices)
        disconnected.vertices.emplace_back(point.x()+10,point.y(),point.z());
    for (const auto& triangle : valid.mesh->triangles)
        disconnected.triangles.push_back({triangle[0]+offset,triangle[1]+offset,triangle[2]+offset});
    EXPECT_EQ(exporting::validateExportMesh(std::move(disconnected)).status,
        exporting::MeshStatus::Unsupported);
    model.setResultFeatureId(extrude);
    EXPECT_TRUE(exporting::buildExportMesh(model,0).succeeded());
    model.setExtrudeDistance(extrude,5);
    EXPECT_EQ(exporting::buildExportMesh(model,0).status,
        exporting::MeshStatus::InvalidInput);
}
}
