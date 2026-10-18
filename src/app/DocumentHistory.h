#pragma once

#include "features/FeatureModel.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace microsw
{
class DocumentHistory
{
public:
    static constexpr std::size_t maximumUndoSteps = 100;

    explicit DocumentHistory(const features::FeatureModel& initial);
    void reset(const features::FeatureModel& model, bool saved);
    void record(const features::FeatureModel& model);
    [[nodiscard]] bool canUndo() const noexcept;
    [[nodiscard]] bool canRedo() const noexcept;
    [[nodiscard]] bool undo(features::FeatureModel& model);
    [[nodiscard]] bool redo(features::FeatureModel& model);
    void markSaved(const features::FeatureModel& model);
    [[nodiscard]] bool isDirty() const noexcept;
    [[nodiscard]] std::size_t undoCount() const noexcept { return cursor_; }
    [[nodiscard]] std::size_t redoCount() const noexcept { return entries_.size() - cursor_ - 1; }
    [[nodiscard]] const std::optional<features::FeatureModelCheckpoint>& savedCheckpoint() const noexcept
        { return savedCheckpoint_; }

private:
    struct Entry { std::uint64_t stateId; features::FeatureModelCheckpoint checkpoint; };
    std::vector<Entry> entries_;
    std::size_t cursor_{};
    std::uint64_t nextStateId_{1};
    std::optional<std::uint64_t> savedStateId_;
    std::optional<features::FeatureModelCheckpoint> savedCheckpoint_;
};
}
