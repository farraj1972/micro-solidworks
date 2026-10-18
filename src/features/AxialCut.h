#pragma once

#include "core/math/Vector3.h"

namespace microsw::features
{

enum class AxialCutAlignment
{
    Parallel,
    AntiParallel,
    Unsupported
};

// Both vectors are directions. The existing geometric tolerance is applied to
// their normalized cross-product magnitude, avoiding exact angle comparisons.
[[nodiscard]] AxialCutAlignment classifyAxialCut(
    const math::Vector3& cutDirection,
    const math::Vector3& targetAxis);

}
