#include "viewer/ReferenceGrid.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace microsw::viewer
{

ReferenceGrid::ReferenceGrid(math::Scalar halfExtent, math::Scalar spacing)
    : halfExtent_{halfExtent}, spacing_{spacing}
{
    if (!std::isfinite(halfExtent) || !std::isfinite(spacing)
        || halfExtent <= 0.0 || spacing <= 0.0)
    {
        throw std::invalid_argument{"Grid extent and spacing must be positive and finite"};
    }

    // Resource guard, not a UX/architectural grid-size limit: cap CPU vertex
    // storage at 64 MiB rather than attempting an unbounded allocation.
    constexpr std::size_t storageBudget = 64 * 1024 * 1024;
    const auto maxCount = std::min(vertices_.max_size(), storageBudget / sizeof(math::Vector3)) / 8;
    const auto intervals = std::floor(halfExtent / spacing);
    if (!std::isfinite(intervals) || intervals > static_cast<math::Scalar>(maxCount))
    {
        throw std::length_error{"Grid exceeds vertex storage budget"};
    }
    auto count = static_cast<std::size_t>(intervals);
    // Division can round up at a boundary. Do not emit a line beyond the extent.
    if (count > 0 && static_cast<math::Scalar>(count) * spacing > halfExtent)
        --count;

    // Each positive index supplies +/- positions in both directions: 8 vertices.
    // The bounded count makes the integer conversion, multiplication and loop safe.
    vertices_.reserve(count * 8);
    for (std::size_t index = 1; index <= count; ++index)
    {
        const auto distance = static_cast<math::Scalar>(index) * spacing;
        for (const auto position : {-distance, distance})
        {
            vertices_.emplace_back(position, -halfExtent, 0.0);
            vertices_.emplace_back(position, halfExtent, 0.0);
            vertices_.emplace_back(-halfExtent, position, 0.0);
            vertices_.emplace_back(halfExtent, position, 0.0);
        }
    }
}

}
