#pragma once

#include <cstdint>
#include <limits>

namespace microsw::features
{
class FeatureId
{
public:
    using Value = std::uint32_t;
    static constexpr Value invalidValue = std::numeric_limits<Value>::max();
    constexpr FeatureId() noexcept = default;
    explicit constexpr FeatureId(Value value) noexcept : value_{value} {}
    [[nodiscard]] constexpr Value value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool isValid() const noexcept { return value_ != invalidValue; }
    friend constexpr bool operator==(FeatureId, FeatureId) noexcept = default;
    friend constexpr auto operator<=>(FeatureId, FeatureId) noexcept = default;
private:
    Value value_{invalidValue};
};
}
