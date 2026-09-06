#pragma once

#include "presentation/VisualEntityId.h"

#include <optional>

namespace microsw::viewer
{
enum class VisualState { Normal, Hovered, Selected };

[[nodiscard]] inline VisualState visualStateFor(presentation::VisualEntityId id,
    std::optional<presentation::VisualEntityId> hovered,
    std::optional<presentation::VisualEntityId> selected) noexcept
{
    if (selected == id) return VisualState::Selected;
    if (hovered == id) return VisualState::Hovered;
    return VisualState::Normal;
}

// A filter partitions one presentation snapshot into mutually exclusive batches.
// An absent filter in an adapter preserves its original all-entities contract.
struct VisualStateFilter
{
    VisualState state;
    std::optional<presentation::VisualEntityId> hovered;
    std::optional<presentation::VisualEntityId> selected;

    [[nodiscard]] bool accepts(presentation::VisualEntityId id) const noexcept
    {
        return visualStateFor(id, hovered, selected) == state;
    }
};
}
