#include "viewer/PresentedPoints.h"

#include "core/geometry/Point3.h"
#include "presentation/GeometryPresentation.h"
#include "presentation/WorldGeometry.h"

#include <variant>

namespace microsw::viewer
{
std::vector<math::Vector3> presentedPointVertices(
    const presentation::GeometryPresentation& presentation,
    std::optional<VisualStateFilter> filter)
{
    std::vector<math::Vector3> vertices;
    vertices.reserve(presentation.size());
    for (const auto& entity : presentation.entities())
    {
        if (filter && !filter->accepts(entity.id())) continue;
        if (!std::holds_alternative<geometry::Point3>(entity.geometry())) continue;
        const auto world = presentation::worldGeometry(entity);
        if (const auto* point = std::get_if<geometry::Point3>(&world))
            vertices.emplace_back(point->x(), point->y(), point->z());
    }
    return vertices;
}
}
