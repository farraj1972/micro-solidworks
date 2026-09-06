#include "viewer/PresentedPoints.h"

#include "core/geometry/Point3.h"
#include "presentation/GeometryPresentation.h"

#include <variant>

namespace microsw::viewer
{
std::vector<math::Vector3> presentedPointVertices(
    const presentation::GeometryPresentation& presentation)
{
    std::vector<math::Vector3> vertices;
    vertices.reserve(presentation.size());
    for (const auto& entity : presentation.entities())
        if (const auto* point = std::get_if<geometry::Point3>(&entity.geometry()))
            vertices.emplace_back(point->x(), point->y(), point->z());
    return vertices;
}
}
