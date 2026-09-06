#include "viewer/PresentedLines.h"

#include "core/geometry/Line3.h"
#include "presentation/GeometryPresentation.h"

#include <cmath>
#include <stdexcept>
#include <variant>

namespace microsw::viewer
{
std::vector<math::Vector3> presentedLineVertices(
    const presentation::GeometryPresentation& presentation,
    const LinePresentationContext& context,
    std::optional<VisualStateFilter> filter)
{
    if (!std::isfinite(context.visibleScale) || context.visibleScale <= 0)
        throw std::invalid_argument{"Line presentation scale must be positive and finite"};
    constexpr math::Scalar extentMultiplier = 3.0;
    const math::Scalar extent = context.visibleScale * extentMultiplier;
    if (!std::isfinite(extent))
        throw std::overflow_error{"Line presentation extent is not representable"};

    const geometry::Point3 viewCenter{
        context.viewCenter.x(), context.viewCenter.y(), context.viewCenter.z()};
    std::vector<math::Vector3> vertices;
    vertices.reserve(presentation.size() * 2);
    for (const auto& entity : presentation.entities())
    {
        if (filter && !filter->accepts(entity.id())) continue;
        if (const auto* line = std::get_if<geometry::Line3>(&entity.geometry()))
        {
            const geometry::Point3 center = geometry::closestPoint(*line, viewCenter);
            const geometry::Point3 negative = center - line->direction() * extent;
            const geometry::Point3 positive = center + line->direction() * extent;
            vertices.emplace_back(negative.x(), negative.y(), negative.z());
            vertices.emplace_back(positive.x(), positive.y(), positive.z());
        }
    }
    return vertices;
}
}
