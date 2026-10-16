#pragma once
#include "persistence/Persistence.h"
#include <filesystem>
#include <optional>
namespace microsw {
class PersistenceSession {
public:
 PersistenceSession()=default;
 explicit PersistenceSession(features::FeatureModel model):model_{std::move(model)}{}
 [[nodiscard]] features::FeatureModel& model() noexcept{return model_;}
 [[nodiscard]] const features::FeatureModel& model() const noexcept{return model_;}
 [[nodiscard]] const std::optional<std::filesystem::path>& currentFilePath()const noexcept{return currentFilePath_;}
 void newDocument();
 [[nodiscard]] persistence::SaveResult save() noexcept;
 [[nodiscard]] persistence::SaveResult saveAs(const std::filesystem::path&) noexcept;
 [[nodiscard]] persistence::OpenStatus open(const std::filesystem::path&,std::string* diagnostic=nullptr) noexcept;
private:
 features::FeatureModel model_;
 std::optional<std::filesystem::path> currentFilePath_;
}; }
