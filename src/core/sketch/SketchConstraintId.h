#pragma once

#include <cstdint>
#include <limits>

namespace microsw::sketch
{

class SketchConstraintId
{
public:
    using Value = std::uint32_t;
    static constexpr Value invalidValue = std::numeric_limits<Value>::max();

    constexpr SketchConstraintId() noexcept = default;
    explicit constexpr SketchConstraintId(Value value) noexcept : value_{value} {}

    [[nodiscard]] constexpr Value value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool isValid() const noexcept { return value_ != invalidValue; }
    friend constexpr bool operator==(SketchConstraintId, SketchConstraintId) noexcept = default;

private:
    Value value_{invalidValue};
};

}
