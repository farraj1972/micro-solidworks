#pragma once

#include "presentation/VisualEntityId.h"

#include <optional>

namespace microsw::viewer
{
// Interaction identity persists until an eligible click changes it.
// Valid only for the observed presentation's lifetime; no CAD/persistence ownership.
class SelectionState
{
public:
    [[nodiscard]] const std::optional<presentation::VisualEntityId>& selected() const noexcept
    {
        return selected_;
    }
    void select(presentation::VisualEntityId id) noexcept { selected_ = id; }
    void clear() noexcept { selected_.reset(); }

private:
    std::optional<presentation::VisualEntityId> selected_;
};
}
