#include "app/DocumentHistory.h"

namespace microsw
{
DocumentHistory::DocumentHistory(const features::FeatureModel& initial) { reset(initial, true); }

void DocumentHistory::reset(const features::FeatureModel& model, bool saved)
{
    entries_.clear();
    entries_.push_back({nextStateId_++, model.checkpoint()});
    cursor_ = 0;
    if (saved) markSaved(model);
    else { savedStateId_.reset(); savedCheckpoint_.reset(); }
}

void DocumentHistory::record(const features::FeatureModel& model)
{
    entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(cursor_ + 1), entries_.end());
    entries_.push_back({nextStateId_++, model.checkpoint()});
    cursor_ = entries_.size() - 1;
    constexpr auto maximumEntries = maximumUndoSteps + 1;
    if (entries_.size() > maximumEntries)
    {
        entries_.erase(entries_.begin());
        --cursor_;
    }
}

bool DocumentHistory::canUndo() const noexcept { return cursor_ > 0; }
bool DocumentHistory::canRedo() const noexcept { return cursor_ + 1 < entries_.size(); }

bool DocumentHistory::undo(features::FeatureModel& model)
{
    if (!canUndo()) return false;
    --cursor_;
    model.restoreCheckpoint(entries_[cursor_].checkpoint);
    return true;
}

bool DocumentHistory::redo(features::FeatureModel& model)
{
    if (!canRedo()) return false;
    ++cursor_;
    model.restoreCheckpoint(entries_[cursor_].checkpoint);
    return true;
}

void DocumentHistory::markSaved(const features::FeatureModel& model)
{
    savedStateId_ = entries_[cursor_].stateId;
    savedCheckpoint_ = model.checkpoint();
}

bool DocumentHistory::isDirty() const noexcept
{
    return !savedStateId_ || *savedStateId_ != entries_[cursor_].stateId;
}
}
