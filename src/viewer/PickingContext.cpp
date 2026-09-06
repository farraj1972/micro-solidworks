#include "viewer/PickingContext.h"

#include <cmath>
#include <stdexcept>

namespace microsw::viewer
{
void PickingContext::validate() const
{
    for (const auto value : {width, height, mouseX, mouseY, verticalFov,
                            nearPlane, farPlane, tolerancePixels})
        if (!std::isfinite(value))
            throw std::invalid_argument{"Picking context values must be finite"};
    if (width <= 0 || height <= 0 || tolerancePixels <= 0
        || verticalFov <= 0 || verticalFov >= std::numbers::pi_v<math::Scalar>
        || nearPlane <= 0 || farPlane <= nearPlane)
        throw std::invalid_argument{"Invalid picking viewport, tolerance or projection"};
    const auto aspect = aspectRatio();
    if (!std::isfinite(aspect) || aspect <= 0)
        throw std::invalid_argument{"Picking aspect must be positive and finite"};
    if (projection.mode() != ProjectionMode::Perspective
        && projection.mode() != ProjectionMode::Orthographic)
        throw std::invalid_argument{"Unknown picking projection mode"};
    const auto matrix = projection.matrix(verticalFov, aspect, nearPlane, farPlane);
    if (matrix(0, 0) <= 0 || matrix(1, 1) <= 0)
        throw std::overflow_error{"Picking projection scale is not representable"};
}
}
