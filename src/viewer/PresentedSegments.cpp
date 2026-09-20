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
        if (!std::holds_alternative<geometry::Segment3>(entity.geometry())
            && !std::holds_alternative<presentation::Polyline3>(entity.geometry())
            && !std::holds_alternative<presentation::SegmentSet3>(entity.geometry())) continue;
        const auto world = presentation::worldGeometry(entity);
        if (const auto* segment = std::get_if<geometry::Segment3>(&world))
        {
            const auto& a = segment->a();
            const auto& b = segment->b();
            vertices.emplace_back(a.x(), a.y(), a.z());
            vertices.emplace_back(b.x(), b.y(), b.z());
        }
        else if (const auto* polyline = std::get_if<presentation::Polyline3>(&world))
        {
            const auto& points = polyline->points();
            for (std::size_t i = 1; i < points.size(); ++i)
            {
                vertices.emplace_back(points[i - 1].x(), points[i - 1].y(), points[i - 1].z());
                vertices.emplace_back(points[i].x(), points[i].y(), points[i].z());
            }
        }
        else if (const auto* set = std::get_if<presentation::SegmentSet3>(&world))
        {
            for (const auto& segment : set->segments())
            {
                vertices.emplace_back(segment.a().x(), segment.a().y(), segment.a().z());
                vertices.emplace_back(segment.b().x(), segment.b().y(), segment.b().z());
            }
        }
    }
    return vertices;
}
}
