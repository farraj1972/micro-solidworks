#pragma once

#include <filesystem>
#include <optional>

namespace microsw
{
class ApplicationWindow;

[[nodiscard]] std::optional<std::filesystem::path> chooseMicroswFileToOpen(
    const ApplicationWindow& owner);
[[nodiscard]] std::optional<std::filesystem::path> chooseMicroswFileToSave(
    const ApplicationWindow& owner, const std::optional<std::filesystem::path>& currentPath);
[[nodiscard]] std::optional<std::filesystem::path> chooseStlFileToExport(
    const ApplicationWindow& owner);
}
