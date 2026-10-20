#pragma once

#include <filesystem>
#include <string>

namespace microsw::preferences
{
enum class NavigationProfile { OrbitMiddleButton, PanMiddleButton };
enum class SelectionAccent { Orange, Blue };

struct ApplicationPreferences
{
    NavigationProfile navigationProfile{NavigationProfile::OrbitMiddleButton};
    SelectionAccent selectionAccent{SelectionAccent::Orange};
    friend bool operator==(const ApplicationPreferences&, const ApplicationPreferences&) = default;
};

[[nodiscard]] bool valid(const ApplicationPreferences&) noexcept;
[[nodiscard]] std::filesystem::path userPreferencesPath();

enum class LoadStatus { Defaults, Loaded, RecoveredBackup, Invalid, UnsupportedVersion, StorageUnavailable };
struct LoadResult { LoadStatus status{LoadStatus::Defaults}; std::string diagnostic; };
struct ApplyResult { bool success{}; std::string diagnostic; };

// Application-owned user configuration. The injected path makes storage failures
// and recovery testable without touching the actual user's profile.
class PreferencesService
{
public:
    explicit PreferencesService(std::filesystem::path path);
    [[nodiscard]] LoadResult load() noexcept;
    [[nodiscard]] const ApplicationPreferences& current() const noexcept { return current_; }
    [[nodiscard]] ApplyResult apply(const ApplicationPreferences& candidate,
                                    bool confirmFutureVersionReset = false) noexcept;
    [[nodiscard]] static constexpr ApplicationPreferences defaults() noexcept { return {}; }
    [[nodiscard]] bool futureVersionNeedsReset() const noexcept { return futureVersion_; }

private:
    std::filesystem::path path_;
    ApplicationPreferences current_{};
    bool futureVersion_{};
};
}
