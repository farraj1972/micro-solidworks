#pragma once

#include "features/FeatureModel.h"

#include <filesystem>
#include <string>

namespace microsw::persistence
{
inline constexpr std::uintmax_t maximumFileSize = 64U * 1024U * 1024U;
inline constexpr std::size_t maximumFeatureCount = 100000U;
inline constexpr std::size_t maximumSketchEntityCount = 250000U;
inline constexpr std::size_t maximumSketchConstraintCount = 250000U;

enum class SaveStatus { Success, InvalidPath, SerializationError, IoError };
enum class OpenStatus { SuccessRebuilt, SuccessRebuildFailed, IoError,
    ParseError, UnsupportedVersion, InvalidModel };
struct SaveResult { SaveStatus status{SaveStatus::Success}; std::string diagnostic; };
struct OpenResult { OpenStatus status{OpenStatus::InvalidModel}; std::string diagnostic; features::FeatureModel model; };

[[nodiscard]] SaveResult saveFeatureModel(
    const features::FeatureModel& model, const std::filesystem::path& path) noexcept;
[[nodiscard]] OpenResult openFeatureModel(const std::filesystem::path& path) noexcept;
}
