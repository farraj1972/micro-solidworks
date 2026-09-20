#include "core/sketch/SketchConstraint.h"

#include "core/geometry/GeometricTolerance.h"

#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace microsw::sketch
{
namespace
{
void validateDrivingValue(const SketchConstraintValue& value, math::Scalar drivingValue)
{
    if (!std::isfinite(drivingValue))
        throw std::invalid_argument{"Driving dimension must be finite"};
    if ((std::holds_alternative<LineLength>(value) || std::holds_alternative<CircleRadius>(value))
        && drivingValue <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Length and radius dimensions must exceed geometric tolerance"};
}
}

bool isDrivingDimension(const SketchConstraintValue& value) noexcept
{
    return std::holds_alternative<HorizontalDistance>(value)
        || std::holds_alternative<VerticalDistance>(value)
        || std::holds_alternative<LineLength>(value)
        || std::holds_alternative<CircleRadius>(value);
}

SketchConstraintValue withDrivingValue(
    const SketchConstraintValue& value, math::Scalar drivingValue)
{
    if (!isDrivingDimension(value))
        throw std::invalid_argument{"Constraint is not a driving dimension"};
    validateDrivingValue(value, drivingValue);
    return std::visit([&](const auto& constraint) -> SketchConstraintValue {
        using T = std::decay_t<decltype(constraint)>;
        if constexpr (std::is_same_v<T, HorizontalDistance>)
            return HorizontalDistance{constraint.first, constraint.second, drivingValue};
        else if constexpr (std::is_same_v<T, VerticalDistance>)
            return VerticalDistance{constraint.first, constraint.second, drivingValue};
        else if constexpr (std::is_same_v<T, LineLength>)
            return LineLength{constraint.line, drivingValue};
        else if constexpr (std::is_same_v<T, CircleRadius>)
            return CircleRadius{constraint.radius, drivingValue};
        else throw std::invalid_argument{"Constraint is not a driving dimension"};
    }, value);
}

}
