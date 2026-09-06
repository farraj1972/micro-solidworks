#include "viewer/PickingRay.h"

#include <cmath>
#include <stdexcept>

namespace microsw::viewer
{
geometry::Ray3 makePickingRay(const PickingContext& context)
{
    context.validate();
    const auto x = 2.0 * (context.mouseX / context.width) - 1.0;
    const auto y = 1.0 - 2.0 * (context.mouseY / context.height);
    const auto aspect = context.aspectRatio();
    const auto halfHeight = context.projection.mode() == ProjectionMode::Perspective
        ? std::tan(context.verticalFov / 2.0)
        : context.projection.visibleHeight() / 2.0;
    const auto horizontal = x * halfHeight * aspect;
    const auto vertical = y * halfHeight;
    if (!std::isfinite(horizontal) || !std::isfinite(vertical))
        throw std::overflow_error{"Picking ray image-plane offset is not representable"};

    const auto offset = context.camera.right() * horizontal + context.camera.up() * vertical;
    const auto origin = context.projection.mode() == ProjectionMode::Perspective
        ? context.camera.position() : context.camera.position() + offset;
    const auto direction = context.projection.mode() == ProjectionMode::Perspective
        ? context.camera.forward() + offset : context.camera.forward();
    for (const auto value : {origin.x(), origin.y(), origin.z(),
                            direction.x(), direction.y(), direction.z()})
        if (!std::isfinite(value))
            throw std::overflow_error{"Picking ray is not representable"};
    // hypot avoids overflow in squaredLength for large but finite directions.
    const auto length = std::hypot(direction.x(), direction.y(), direction.z());
    if (!std::isfinite(length) || length <= 0)
        throw std::overflow_error{"Picking ray direction cannot be normalized"};
    return {geometry::Point3{origin.x(), origin.y(), origin.z()},
        math::Vector3{direction.x() / length, direction.y() / length, direction.z() / length}};
}
}
