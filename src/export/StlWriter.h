#pragma once

#include "export/ExportMesh.h"

#include <filesystem>
#include <string>

namespace microsw::exporting
{
enum class StlWriteStatus { Success, InvalidMesh, InvalidPath, IoError };
struct StlWriteResult
{
    StlWriteStatus status{StlWriteStatus::Success};
    std::string diagnostic;
    [[nodiscard]] bool succeeded() const noexcept { return status == StlWriteStatus::Success; }
};

// Binary STL is unitless. Coordinates in this contract are interpreted as mm
// by the caller and must be stated as such in the user-facing export flow.
[[nodiscard]] StlWriteResult writeBinaryStl(const ExportMesh& mesh,
    const std::filesystem::path& destination, const MeshQuality& quality = {}) noexcept;
}
