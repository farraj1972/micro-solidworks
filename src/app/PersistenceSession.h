#pragma once
#include "persistence/Persistence.h"
#include "app/DocumentHistory.h"
#include <filesystem>
#include <optional>
namespace microsw {
class PersistenceSession {
public:
 PersistenceSession();
 explicit PersistenceSession(features::FeatureModel model);
 [[nodiscard]] features::FeatureModel& model() noexcept{return model_;}
 [[nodiscard]] const features::FeatureModel& model() const noexcept{return model_;}
 [[nodiscard]] const std::optional<std::filesystem::path>& currentFilePath()const noexcept{return currentFilePath_;}
 [[nodiscard]] bool isDirty() const noexcept{return history_.isDirty();}
 [[nodiscard]] bool canUndo() const noexcept{return history_.canUndo();}
 [[nodiscard]] bool canRedo() const noexcept{return history_.canRedo();}
 void recordEdit(){history_.record(model_);}
 [[nodiscard]] bool undo();
 [[nodiscard]] bool redo();
 void newDocument();
 [[nodiscard]] persistence::SaveResult save() noexcept;
 [[nodiscard]] persistence::SaveResult saveAs(const std::filesystem::path&) noexcept;
 [[nodiscard]] persistence::OpenStatus open(const std::filesystem::path&,std::string* diagnostic=nullptr) noexcept;
private:
 features::FeatureModel model_;
 DocumentHistory history_;
 std::optional<std::filesystem::path> currentFilePath_;
}; }
