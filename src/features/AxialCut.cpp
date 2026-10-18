#include "features/AxialCut.h"

#include "core/geometry/GeometricTolerance.h"

#include <cmath>
#include <stdexcept>

namespace microsw::features
{

AxialCutAlignment classifyAxialCut(const math::Vector3& cutDirection,
                                   const math::Vector3& targetAxis)
{
    const auto cut = cutDirection.normalized();
    const auto target = targetAxis.normalized();
    const auto crossProduct = math::cross(cut, target);
    const auto residual = std::hypot(
        crossProduct.x(), crossProduct.y(), crossProduct.z());
    if (residual > geometry::defaultGeometricTolerance)
        return AxialCutAlignment::Unsupported;
    return math::dot(cut, target) >= 0.0
        ? AxialCutAlignment::Parallel
        : AxialCutAlignment::AntiParallel;
}

}
