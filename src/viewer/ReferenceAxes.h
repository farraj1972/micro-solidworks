#pragma once

#include "core/math/Vector3.h"

#include <array>

namespace microsw::viewer
{

// Positive-axis render aids, not CAD entities. Length is a viewer parameter.
class ReferenceAxes
{
public:
    [[nodiscard]] static constexpr math::Scalar length() noexcept { return 3.0; }
    [[nodiscard]] std::array<math::Vector3, 2> xAxis() const noexcept;
    [[nodiscard]] std::array<math::Vector3, 2> yAxis() const noexcept;
    [[nodiscard]] std::array<math::Vector3, 2> zAxis() const noexcept;
};

}
