#include "app/preferences/ApplicationPreferences.h"

#include <gtest/gtest.h>

#include <chrono>
#include <fstream>

namespace
{
namespace pref = microsw::preferences;

struct TemporaryPreferences
{
    std::filesystem::path folder = std::filesystem::temp_directory_path()
        / ("microsw-preferences-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::path file = folder / "preferences.json";
    ~TemporaryPreferences() { std::error_code error; std::filesystem::remove_all(folder, error); }
    void write(const std::string& text)
    {
        std::filesystem::create_directories(folder);
        std::ofstream stream{file}; stream << text;
    }
};
}

TEST(ApplicationPreferences, DefaultsAndMissingFile)
{
    TemporaryPreferences temporary;
    pref::PreferencesService service{temporary.file};
    EXPECT_EQ(service.load().status, pref::LoadStatus::Defaults);
    EXPECT_EQ(service.current(), pref::PreferencesService::defaults());
    EXPECT_EQ(service.current().navigationProfile, pref::NavigationProfile::OrbitMiddleButton);
    EXPECT_EQ(service.current().selectionAccent, pref::SelectionAccent::Orange);
}

TEST(ApplicationPreferences, ApplyPersistsBothTypedChoicesAcrossReload)
{
    TemporaryPreferences temporary;
    pref::PreferencesService service{temporary.file};
    const pref::ApplicationPreferences alternative{pref::NavigationProfile::PanMiddleButton,
                                                   pref::SelectionAccent::Blue};
    ASSERT_TRUE(service.apply(alternative).success);
    pref::PreferencesService restarted{temporary.file};
    EXPECT_EQ(restarted.load().status, pref::LoadStatus::Loaded);
    EXPECT_EQ(restarted.current(), alternative);
    ASSERT_TRUE(restarted.apply(pref::PreferencesService::defaults()).success);
    EXPECT_EQ(restarted.current(), pref::PreferencesService::defaults());
}

TEST(ApplicationPreferences, InvalidCandidateNeverReplacesCurrentOrFile)
{
    TemporaryPreferences temporary;
    pref::PreferencesService service{temporary.file};
    ASSERT_TRUE(service.apply({}).success);
    const pref::ApplicationPreferences invalid{static_cast<pref::NavigationProfile>(99), pref::SelectionAccent::Blue};
    EXPECT_FALSE(service.apply(invalid).success);
    EXPECT_EQ(service.current(), pref::PreferencesService::defaults());
    pref::PreferencesService restarted{temporary.file};
    EXPECT_EQ(restarted.load().status, pref::LoadStatus::Loaded);
    EXPECT_EQ(restarted.current(), pref::PreferencesService::defaults());
}

TEST(ApplicationPreferences, PartialUnknownAndInvalidFieldsUseDefaults)
{
    TemporaryPreferences temporary;
    temporary.write(R"({"schemaVersion":1,"navigationProfile":"PanMiddleButton","futureField":1})");
    pref::PreferencesService service{temporary.file};
    EXPECT_EQ(service.load().status, pref::LoadStatus::Loaded);
    EXPECT_EQ(service.current().navigationProfile, pref::NavigationProfile::PanMiddleButton);
    EXPECT_EQ(service.current().selectionAccent, pref::SelectionAccent::Orange);
    temporary.write(R"({"schemaVersion":1,"navigationProfile":"bad","selectionAccent":"Blue"})");
    EXPECT_EQ(service.load().status, pref::LoadStatus::Invalid);
    EXPECT_EQ(service.current().navigationProfile, pref::NavigationProfile::OrbitMiddleButton);
    EXPECT_EQ(service.current().selectionAccent, pref::SelectionAccent::Blue);
}

TEST(ApplicationPreferences, CorruptPrimaryCanRecoverBackup)
{
    TemporaryPreferences temporary;
    temporary.write("not json");
    pref::PreferencesService service{temporary.file};
    EXPECT_EQ(service.load().status, pref::LoadStatus::Invalid);
    EXPECT_EQ(service.current(), pref::PreferencesService::defaults());
    std::ofstream backup{temporary.file.string() + ".bak"};
    backup << R"({"schemaVersion":1,"selectionAccent":"Blue"})";
    backup.close();
    EXPECT_EQ(service.load().status, pref::LoadStatus::RecoveredBackup);
    EXPECT_EQ(service.current().selectionAccent, pref::SelectionAccent::Blue);
}

TEST(ApplicationPreferences, FutureVersionRequiresExplicitReset)
{
    TemporaryPreferences temporary;
    temporary.write(R"({"schemaVersion":2,"selectionAccent":"Blue"})");
    pref::PreferencesService service{temporary.file};
    EXPECT_EQ(service.load().status, pref::LoadStatus::UnsupportedVersion);
    EXPECT_TRUE(service.futureVersionNeedsReset());
    EXPECT_FALSE(service.apply({}).success);
    pref::PreferencesService stillFuture{temporary.file};
    EXPECT_EQ(stillFuture.load().status, pref::LoadStatus::UnsupportedVersion);
    EXPECT_TRUE(service.apply({}, true).success);
    EXPECT_EQ(stillFuture.load().status, pref::LoadStatus::Loaded);
}

TEST(ApplicationPreferences, UnwritableDestinationDoesNotPublish)
{
    TemporaryPreferences temporary;
    std::filesystem::create_directories(temporary.file);
    pref::PreferencesService service{temporary.file};
    EXPECT_FALSE(service.apply({pref::NavigationProfile::PanMiddleButton,
                                pref::SelectionAccent::Blue}).success);
    EXPECT_EQ(service.current(), pref::PreferencesService::defaults());
}
