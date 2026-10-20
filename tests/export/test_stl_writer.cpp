#include "export/StlWriter.h"
#include "core/topology/CuboidBuilder.h"

#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace
{
using namespace microsw;

std::uint32_t decodeU32(const std::vector<unsigned char>& bytes, std::size_t offset)
{
    return static_cast<std::uint32_t>(bytes.at(offset))
        | static_cast<std::uint32_t>(bytes.at(offset+1)) << 8
        | static_cast<std::uint32_t>(bytes.at(offset+2)) << 16
        | static_cast<std::uint32_t>(bytes.at(offset+3)) << 24;
}

float decodeFloat(const std::vector<unsigned char>& bytes, std::size_t offset)
{
    return std::bit_cast<float>(decodeU32(bytes,offset));
}

std::filesystem::path testPath()
{
    return std::filesystem::temp_directory_path()
        / ("microsw-export-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()) + ".stl");
}

TEST(StlWriter, IndependentBinaryParseVerifiesBoundsNormalsAndVolume)
{
    const auto meshResult = exporting::buildExportMesh(topology::makeCuboid(2,3,4));
    ASSERT_TRUE(meshResult.succeeded()) << meshResult.diagnostic;
    const auto path = testPath();
    ASSERT_TRUE(exporting::writeBinaryStl(*meshResult.mesh,path).succeeded());
    std::ifstream file(path,std::ios::binary);
    ASSERT_TRUE(file);
    const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}};
    const auto count = decodeU32(bytes,80);
    ASSERT_EQ(count,meshResult.mesh->triangles.size());
    ASSERT_EQ(bytes.size(),84u+50u*count);
    double volume = 0;
    double minX = 1e99, maxX = -1e99, minY = 1e99, maxY = -1e99, minZ = 1e99, maxZ = -1e99;
    for (std::size_t i=0; i<count; ++i)
    {
        const auto offset=84+50*i;
        const math::Vector3 normal{decodeFloat(bytes,offset),decodeFloat(bytes,offset+4),
            decodeFloat(bytes,offset+8)};
        std::array<geometry::Point3,3> points;
        for (int corner=0;corner<3;++corner)
        {
            const auto p=offset+12+12*corner;
            points[corner]={decodeFloat(bytes,p),decodeFloat(bytes,p+4),decodeFloat(bytes,p+8)};
            minX=std::min(minX,points[corner].x());maxX=std::max(maxX,points[corner].x());
            minY=std::min(minY,points[corner].y());maxY=std::max(maxY,points[corner].y());
            minZ=std::min(minZ,points[corner].z());maxZ=std::max(maxZ,points[corner].z());
        }
        const auto cross=math::cross(points[1]-points[0],points[2]-points[0]);
        EXPECT_GT(math::dot(cross,normal),0);
        EXPECT_NEAR(normal.length(),1,1e-6);
        volume+=math::dot(points[0]-geometry::Point3{0,0,0},cross)/6;
    }
    EXPECT_NEAR(volume,24,1e-5);
    EXPECT_NEAR(maxX-minX,2,1e-6);EXPECT_NEAR(maxY-minY,3,1e-6);
    EXPECT_NEAR(maxZ-minZ,4,1e-6);
    std::error_code ignored;std::filesystem::remove(path,ignored);
}

TEST(StlWriter, RepeatedWriteIsDeterministicAndFailurePreservesDestination)
{
    const auto mesh = *exporting::buildExportMesh(topology::makeCuboid(2,3,4)).mesh;
    const auto path=testPath();
    ASSERT_TRUE(exporting::writeBinaryStl(mesh,path).succeeded());
    std::ifstream first(path,std::ios::binary);
    const std::vector<char> original{std::istreambuf_iterator<char>{first},
        std::istreambuf_iterator<char>{}};
    first.close();
    ASSERT_TRUE(exporting::writeBinaryStl(mesh,path).succeeded());
    std::ifstream second(path,std::ios::binary);
    const std::vector<char> repeated{std::istreambuf_iterator<char>{second},
        std::istreambuf_iterator<char>{}};
    second.close();
    EXPECT_EQ(repeated,original);
    auto damaged=mesh;
    damaged.triangles.pop_back();
    EXPECT_EQ(exporting::writeBinaryStl(damaged,path).status,exporting::StlWriteStatus::InvalidMesh);
    std::ifstream kept(path,std::ios::binary);
    EXPECT_EQ((std::vector<char>{std::istreambuf_iterator<char>{kept},
        std::istreambuf_iterator<char>{}}),original);
    EXPECT_EQ(exporting::writeBinaryStl(mesh,{}).status,exporting::StlWriteStatus::InvalidPath);
    const auto missingParent=path.parent_path() / "microsw-export-nonexistent-parent" / "part.stl";
    EXPECT_EQ(exporting::writeBinaryStl(mesh,missingParent).status,
        exporting::StlWriteStatus::IoError);
    std::error_code ignored;std::filesystem::remove(path,ignored);
}

TEST(StlWriter, ReferencePlateSurvivesIndependentBinaryInspection)
{
    features::FeatureModel model;
    sketch::Sketch base;
    (void)base.addRectangle({-40,-25},{40,25});
    (void)base.addCircle({{-22,0},4});
    const auto extrude=model.addExtrude(model.addSketch(std::move(base)),8);
    sketch::Sketch pocket;
    (void)pocket.addRectangle({7,-7},{25,7});
    const auto pocketId=model.addFaceSketch(std::move(pocket),extrude,
        features::ExtrudeCapRole::EndCap);
    (void)model.addCutExtrude(pocketId,extrude,2,features::CutDirection::Reverse);
    ASSERT_EQ(model.rebuild().status,features::RebuildStatus::Success);
    const auto mesh=exporting::buildExportMesh(model,0);
    ASSERT_TRUE(mesh.succeeded())<<mesh.diagnostic;
    const auto path=testPath();
    ASSERT_TRUE(exporting::writeBinaryStl(*mesh.mesh,path).succeeded());
    std::ifstream file(path,std::ios::binary);
    ASSERT_TRUE(file);
    const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}};
    const auto count=decodeU32(bytes,80);
    ASSERT_EQ(bytes.size(),84u+50u*count);
    double volume=0,minX=1e99,maxX=-1e99,minY=1e99,maxY=-1e99,minZ=1e99,maxZ=-1e99;
    for(std::size_t i=0;i<count;++i)
    {
        const auto offset=84+50*i;
        std::array<geometry::Point3,3> p;
        for(int c=0;c<3;++c)
        {
            const auto start=offset+12+12*c;
            p[c]={decodeFloat(bytes,start),decodeFloat(bytes,start+4),decodeFloat(bytes,start+8)};
            minX=std::min(minX,p[c].x());maxX=std::max(maxX,p[c].x());
            minY=std::min(minY,p[c].y());maxY=std::max(maxY,p[c].y());
            minZ=std::min(minZ,p[c].z());maxZ=std::max(maxZ,p[c].z());
        }
        const auto cross=math::cross(p[1]-p[0],p[2]-p[0]);
        volume+=math::dot(p[0]-geometry::Point3{0,0,0},cross)/6;
    }
    EXPECT_NEAR(volume,31093.8761403405,1.0);
    EXPECT_NEAR(minX,-40,1e-5);EXPECT_NEAR(maxX,40,1e-5);
    EXPECT_NEAR(minY,-25,1e-5);EXPECT_NEAR(maxY,25,1e-5);
    EXPECT_NEAR(minZ,0,1e-5);EXPECT_NEAR(maxZ,8,1e-5);
    std::error_code ignored;std::filesystem::remove(path,ignored);
}
}
