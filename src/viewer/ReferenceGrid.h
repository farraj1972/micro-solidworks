#pragma once

#include "core/math/Vector3.h"

#include <vector>

namespace microsw::viewer
{

// Neutral XY plane aid, not CAD geometry. ReferenceAxes owns origin orientation.
// Defaults are implementation parameters. Central X/Y lines are omitted.
class ReferenceGrid
{
public:
    // Positive finite parameters required (invalid_argument).
    // Excessive vertex storage is rejected with length_error before allocation.
    explicit ReferenceGrid(math::Scalar halfExtent = 10.0, math::Scalar spacing = 1.0);

    [[nodiscard]] math::Scalar halfExtent() const noexcept { return halfExtent_; }
    [[nodiscard]] math::Scalar spacing() const noexcept { return spacing_; }
    // Consecutive pairs are independent line segments; generated only at construction.
    // spacing > halfExtent produces an empty grid because central lines are omitted.
    [[nodiscard]] const std::vector<math::Vector3>& vertices() const noexcept { return vertices_; }

private:
    math::Scalar halfExtent_;
    math::Scalar spacing_;
    std::vector<math::Vector3> vertices_;
};

}
