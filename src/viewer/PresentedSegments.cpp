#include "viewer/PresentedSegments.h"

#include "core/geometry/Segment3.h"
#include "presentation/GeometryPresentation.h"
#include "presentation/WorldGeometry.h"

#include <variant>

namespace microsw::viewer
{
std::vector<math::Vector3> presentedSegmentVertices(
    const presentation::GeometryPresentation& presentation,
    std::optional<VisualStateFilter> filter)
{
    std::vector<math::Vector3> vertices;
    vertices.reserve(presentation.size() * 2);
    for (const auto& entity : presentation.entities())
    {
        if (filter && !filter->accepts(entity.id())) continue;
        if (!std::holds_alternative<geometry::Segment3>(entity.geometry())) continue;
        const auto world = presentation::worldGeometry(entity);
        if (const auto* segment = std::get_if<geometry::Segment3>(&world))
        {
            const auto& a = segment->a();
            const auto& b = segment->b();
            vertices.emplace_back(a.x(), a.y(), a.z());
            vertices.emplace_back(b.x(), b.y(), b.z());
        }
    }
    return vertices;
}
}
