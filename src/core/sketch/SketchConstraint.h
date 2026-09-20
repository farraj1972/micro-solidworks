#pragma once

#include "core/math/Scalar.h"
#include "core/sketch/SketchConstraintId.h"
#include "core/sketch/SketchEntityId.h"

#include <variant>

namespace microsw::sketch
{

enum class SubElementKind { LineStart, LineEnd, LineBody, CircleCenter, CircleRadius };

struct SketchElementRef
{
    SketchEntityId entity;
    SubElementKind kind;
    friend constexpr bool operator==(const SketchElementRef&, const SketchElementRef&) noexcept = default;
};

struct Coincident { SketchElementRef first; SketchElementRef second; };
struct Horizontal { SketchElementRef line; };
struct Vertical { SketchElementRef line; };
struct Parallel { SketchElementRef first; SketchElementRef second; };
struct Perpendicular { SketchElementRef first; SketchElementRef second; };
struct HorizontalDistance { SketchElementRef first; SketchElementRef second; math::Scalar value; };
struct VerticalDistance { SketchElementRef first; SketchElementRef second; math::Scalar value; };
struct LineLength { SketchElementRef line; math::Scalar value; };
struct CircleRadius { SketchElementRef radius; math::Scalar value; };

using SketchConstraintValue = std::variant<Coincident, Horizontal, Vertical, Parallel,
    Perpendicular, HorizontalDistance, VerticalDistance, LineLength, CircleRadius>;

class SketchConstraint
{
public:
    SketchConstraint(SketchConstraintId id, SketchConstraintValue value)
        : id_{id}, value_{std::move(value)} {}

    [[nodiscard]] SketchConstraintId id() const noexcept { return id_; }
    [[nodiscard]] const SketchConstraintValue& value() const noexcept { return value_; }

private:
    SketchConstraintId id_;
    SketchConstraintValue value_;
};

[[nodiscard]] bool isDrivingDimension(const SketchConstraintValue& value) noexcept;
[[nodiscard]] SketchConstraintValue withDrivingValue(
    const SketchConstraintValue& value, math::Scalar drivingValue);

}
