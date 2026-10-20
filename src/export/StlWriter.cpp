#include "export/StlWriter.h"

#include <array>
#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace microsw::exporting
{
namespace
{
void writeU32(std::ostream& out, std::uint32_t value)
{
    const std::array<char,4> bytes{static_cast<char>(value), static_cast<char>(value>>8),
        static_cast<char>(value>>16), static_cast<char>(value>>24)};
    out.write(bytes.data(), bytes.size());
}

void writeFloat(std::ostream& out, float value)
{
    writeU32(out, std::bit_cast<std::uint32_t>(value));
}

bool installFile(const std::filesystem::path& temp, const std::filesystem::path& destination)
{
#ifdef _WIN32
    return MoveFileExW(temp.c_str(), destination.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::error_code error;
    std::filesystem::rename(temp, destination, error);
    return !error;
#endif
}
}

StlWriteResult writeBinaryStl(const ExportMesh& mesh,
    const std::filesystem::path& destination, const MeshQuality& quality) noexcept
{
    if (destination.empty() || destination.filename().empty())
        return {StlWriteStatus::InvalidPath, "Choose an STL destination"};
    try
    {
        const auto validated = validateExportMesh(mesh, quality);
        if (!validated.succeeded()) return {StlWriteStatus::InvalidMesh, validated.diagnostic};
        if (mesh.triangles.size() > std::numeric_limits<std::uint32_t>::max())
            return {StlWriteStatus::InvalidMesh, "STL triangle count exceeds format limit"};
        auto temp = destination;
        temp += L".tmp-" + std::to_wstring(
            std::chrono::steady_clock::now().time_since_epoch().count());
        const auto cleanup = [&] { std::error_code ignored; std::filesystem::remove(temp, ignored); };
        {
            std::ofstream out(temp, std::ios::binary | std::ios::trunc);
            if (!out) return {StlWriteStatus::IoError, "Cannot create temporary STL file"};
            std::array<char,80> header{};
            const std::string label = "Micro SolidWorks binary STL; coordinates in mm";
            std::copy(label.begin(), label.end(), header.begin());
            out.write(header.data(), header.size());
            writeU32(out, static_cast<std::uint32_t>(mesh.triangles.size()));
            for (const auto& triangle : mesh.triangles)
            {
                const auto& a = mesh.vertices[triangle[0]];
                const auto& b = mesh.vertices[triangle[1]];
                const auto& c = mesh.vertices[triangle[2]];
                const auto normal = math::cross(b-a,c-a).normalized();
                for (const double value : {normal.x(),normal.y(),normal.z(),
                    a.x(),a.y(),a.z(), b.x(),b.y(),b.z(), c.x(),c.y(),c.z()})
                {
                    if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<float>::max())
                    {
                        out.close(); cleanup();
                        return {StlWriteStatus::InvalidMesh, "Mesh coordinate exceeds STL float range"};
                    }
                    writeFloat(out, static_cast<float>(value));
                }
                const std::array<char,2> attribute{};
                out.write(attribute.data(), attribute.size());
            }
            out.flush();
            if (!out) { out.close(); cleanup(); return {StlWriteStatus::IoError, "STL write failed"}; }
        }
        std::error_code error;
        const auto size = std::filesystem::file_size(temp, error);
        if (error || size != 84 + 50 * mesh.triangles.size())
        {
            cleanup();
            return {StlWriteStatus::IoError, "Temporary STL size verification failed"};
        }
        if (!installFile(temp,destination))
        {
            cleanup();
            return {StlWriteStatus::IoError, "Cannot replace STL destination"};
        }
        return {};
    }
    catch (const std::exception& error)
    {
        return {StlWriteStatus::IoError, error.what()};
    }
}
}
