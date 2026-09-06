#pragma once

#include "presentation/VisualEntityId.h"

#include <optional>

namespace microsw::viewer
{
// Transient identity only; valid for the lifetime of the observed collection.
// No geometry ownership, persistent identity or cached picking result.
class HoverState
{
public:
    [[nodiscard]] const std::optional<presentation::VisualEntityId>& hovered() const noexcept
    {
        return hovered_;
    }
    void update(std::optional<presentation::VisualEntityId> hovered) noexcept
    {
        hovered_ = hovered;
    }
    void clear() noexcept { hovered_.reset(); }

private:
    std::optional<presentation::VisualEntityId> hovered_;
};
}
