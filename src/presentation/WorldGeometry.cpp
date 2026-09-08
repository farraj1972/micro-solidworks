#include "presentation/WorldGeometry.h"
#include "core/math/Transformations.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace microsw::presentation
{
namespace
{
void requireFinite(const math::Vector3& value)
{
    if (!std::isfinite(value.x()) || !std::isfinite(value.y()) || !std::isfinite(value.z()))
        throw std::overflow_error{"World geometry is not representable"};
}
geometry::Point3 worldPoint(const math::Matrix4& matrix, const geometry::Point3& point)
{
    const auto value = math::transformPoint(matrix, {point.x(), point.y(), point.z()});
    requireFinite(value);
    return {value.x(), value.y(), value.z()};
}
}

PresentedGeometry worldGeometry(const VisualEntity& entity)
{
    const auto matrix = entity.transform().matrix();
    return std::visit([&matrix](const auto& local) -> PresentedGeometry
    {
        using Geometry = std::decay_t<decltype(local)>;
        if constexpr (std::is_same_v<Geometry, geometry::Point3>)
            return worldPoint(matrix, local);
        else if constexpr (std::is_same_v<Geometry, geometry::Segment3>)
            return geometry::Segment3{worldPoint(matrix, local.a()), worldPoint(matrix, local.b())};
        else
        {
            const auto direction = math::transformDirection(matrix, local.direction());
            requireFinite(direction);
            // Rescale before normalization: positive tiny/large scale must not
            // trigger Geometry's degeneracy tolerance or squared-length overflow.
            const auto magnitude = std::max({std::abs(direction.x()),
                std::abs(direction.y()), std::abs(direction.z())});
            if (magnitude == 0)
                throw std::overflow_error{"World line direction underflowed"};
            const math::Vector3 rescaled{direction.x() / magnitude,
                direction.y() / magnitude, direction.z() / magnitude};
            return geometry::Line3{worldPoint(matrix, local.origin()), rescaled.normalized()};
        }
    }, entity.geometry());
}
}
