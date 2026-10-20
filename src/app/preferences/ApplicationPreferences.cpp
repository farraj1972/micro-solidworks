#include "app/preferences/ApplicationPreferences.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace microsw::preferences
{
namespace
{
constexpr std::uintmax_t maximumBytes = 64 * 1024;
using Json = nlohmann::ordered_json;

struct Parsed { ApplicationPreferences value; bool fallback{}; std::string diagnostic; };

Parsed parse(const std::filesystem::path& path)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > maximumBytes) throw std::runtime_error{"Preferences file unavailable or too large"};
    std::ifstream stream{path, std::ios::binary};
    if (!stream) throw std::runtime_error{"Cannot read preferences file"};
    Json root = Json::parse(stream);
    if (!root.is_object() || !root.contains("schemaVersion") || !root["schemaVersion"].is_number_integer())
        throw std::runtime_error{"Invalid preferences schema"};
    if (root["schemaVersion"].get<int>() != 1)
        throw std::domain_error{"Unsupported preferences schema version"};
    Parsed result;
    if (root.contains("navigationProfile"))
    {
        const auto& entry = root["navigationProfile"];
        if (entry == "OrbitMiddleButton") result.value.navigationProfile = NavigationProfile::OrbitMiddleButton;
        else if (entry == "PanMiddleButton") result.value.navigationProfile = NavigationProfile::PanMiddleButton;
        else { result.fallback = true; result.diagnostic += "Invalid navigation profile; default used. "; }
    }
    if (root.contains("selectionAccent"))
    {
        const auto& entry = root["selectionAccent"];
        if (entry == "Orange") result.value.selectionAccent = SelectionAccent::Orange;
        else if (entry == "Blue") result.value.selectionAccent = SelectionAccent::Blue;
        else { result.fallback = true; result.diagnostic += "Invalid selection accent; default used."; }
    }
    return result;
}

Json encode(const ApplicationPreferences& value)
{
    return {{"schemaVersion", 1},
        {"navigationProfile", value.navigationProfile == NavigationProfile::OrbitMiddleButton
            ? "OrbitMiddleButton" : "PanMiddleButton"},
        {"selectionAccent", value.selectionAccent == SelectionAccent::Orange ? "Orange" : "Blue"}};
}

bool replaceFile(const std::filesystem::path& temp, const std::filesystem::path& target,
                 const std::filesystem::path& backup)
{
#ifdef _WIN32
    if (std::filesystem::exists(target))
    {
        std::error_code error;
        auto savedBackup = backup; savedBackup += ".previous";
        const bool hadBackup = std::filesystem::exists(backup, error);
        if (error) return false;
        if (hadBackup)
        {
            std::filesystem::copy_file(backup, savedBackup,
                std::filesystem::copy_options::overwrite_existing, error);
            if (error) return false;
            std::filesystem::remove(backup, error);
            if (error) return false;
        }
        if (ReplaceFileW(target.c_str(), temp.c_str(), backup.c_str(), REPLACEFILE_WRITE_THROUGH,
                         nullptr, nullptr) != 0)
        {
            if (hadBackup) std::filesystem::remove(savedBackup, error);
            return true;
        }
        if (hadBackup)
        {
            std::filesystem::copy_file(savedBackup, backup,
                std::filesystem::copy_options::overwrite_existing, error);
            std::filesystem::remove(savedBackup, error);
        }
        return false;
    }
    return MoveFileExW(temp.c_str(), target.c_str(), MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::error_code error;
    if (std::filesystem::exists(target))
        std::filesystem::copy_file(target, backup, std::filesystem::copy_options::overwrite_existing, error);
    if (error) return false;
    std::filesystem::rename(temp, target, error);
    return !error;
#endif
}
}

bool valid(const ApplicationPreferences& value) noexcept
{
    return (value.navigationProfile == NavigationProfile::OrbitMiddleButton
        || value.navigationProfile == NavigationProfile::PanMiddleButton)
        && (value.selectionAccent == SelectionAccent::Orange
            || value.selectionAccent == SelectionAccent::Blue);
}

std::filesystem::path userPreferencesPath()
{
    const char* local = std::getenv("LOCALAPPDATA");
    if (!local || !*local) return {};
    return std::filesystem::path{local} / "MicroSolidWorks" / "preferences.json";
}

PreferencesService::PreferencesService(std::filesystem::path path) : path_{std::move(path)} {}

LoadResult PreferencesService::load() noexcept
{
    current_ = defaults();
    futureVersion_ = false;
    if (path_.empty()) return {LoadStatus::StorageUnavailable, "Local user preferences folder is unavailable"};
    std::error_code error;
    if (!std::filesystem::exists(path_, error))
        return error ? LoadResult{LoadStatus::StorageUnavailable, "Cannot inspect preferences file"}
                     : LoadResult{LoadStatus::Defaults, {}};
    try
    {
        const auto parsed = parse(path_);
        current_ = parsed.value;
        return {parsed.fallback ? LoadStatus::Invalid : LoadStatus::Loaded, parsed.diagnostic};
    }
    catch (const std::domain_error& e)
    {
        futureVersion_ = true;
        return {LoadStatus::UnsupportedVersion, e.what()};
    }
    catch (const std::exception& e)
    {
        const std::string diagnostic = e.what();
        auto backup = path_; backup += ".bak";
        try
        {
            if (std::filesystem::exists(backup))
            {
                const auto recovered = parse(backup);
                current_ = recovered.value;
                return {LoadStatus::RecoveredBackup, "Primary preferences invalid; recovered backup. " + diagnostic};
            }
        }
        catch (const std::exception&) {}
        return {LoadStatus::Invalid, "Preferences defaults used: " + diagnostic};
    }
}

ApplyResult PreferencesService::apply(const ApplicationPreferences& candidate,
                                      bool confirmFutureVersionReset) noexcept
{
    if (!valid(candidate)) return {false, "Unsupported preference value"};
    if (path_.empty()) return {false, "Local user preferences folder is unavailable"};
    if (futureVersion_ && !confirmFutureVersionReset)
        return {false, "A newer preferences file exists. Confirm reset before replacing it."};
    std::filesystem::path temp;
    try
    {
        std::filesystem::create_directories(path_.parent_path());
        temp = path_; temp += ".tmp-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        {
            std::ofstream stream{temp, std::ios::binary | std::ios::trunc};
            if (!stream) throw std::runtime_error{"Cannot create temporary preferences file"};
            stream << encode(candidate).dump(2);
            stream.flush();
            if (!stream) throw std::runtime_error{"Cannot write preferences file"};
        }
        auto backup = path_; backup += ".bak";
        if (!replaceFile(temp, path_, backup))
            throw std::runtime_error{"Cannot replace preferences file"};
        current_ = candidate;
        futureVersion_ = false;
        return {true, {}};
    }
    catch (const std::exception& e)
    {
        std::error_code error;
        if (!temp.empty()) std::filesystem::remove(temp, error);
        return {false, e.what()};
    }
}
}
