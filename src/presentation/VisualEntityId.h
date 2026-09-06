#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace microsw::presentation
{
class VisualEntityId
{
public:
    explicit constexpr VisualEntityId(std::uint64_t value) : value_{value}
    {
        if (value == 0)
            throw std::invalid_argument{"Visual entity ID zero is reserved"};
    }
    [[nodiscard]] constexpr std::uint64_t value() const noexcept { return value_; }
    friend constexpr bool operator==(VisualEntityId, VisualEntityId) noexcept = default;
private:
    std::uint64_t value_;
};

// Process-local and intentionally non-thread-safe. Generated IDs are never
// reused during this generator's lifetime.
class VisualEntityIdGenerator
{
public:
    [[nodiscard]] VisualEntityId generate()
    {
        if (next_ == 0)
            throw std::overflow_error{"Visual entity ID space exhausted"};
        const VisualEntityId result{next_};
        next_ = next_ == std::numeric_limits<std::uint64_t>::max() ? 0 : next_ + 1;
        return result;
    }
private:
    std::uint64_t next_{1};
};
}
