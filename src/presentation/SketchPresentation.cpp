#include "presentation/SketchPresentation.h"

#include <cmath>
#include <algorithm>
#include <numbers>
#include <type_traits>

namespace microsw::presentation
{
namespace
{
PresentedGeometry derive(const sketch::SketchEntity& entity, const sketch::SketchPlane& plane)
{
    return std::visit([&plane](const auto& typed) -> PresentedGeometry
    {
        using Entity = std::decay_t<decltype(typed)>;
        if constexpr (std::is_same_v<Entity, sketch::SketchLine>)
            return geometry::Segment3{plane.toWorld(typed.geometry.a()), plane.toWorld(typed.geometry.b())};
        else
        {
            constexpr std::size_t circleSegments = 64;
            const auto sweep = [&]
            {
                if constexpr (std::is_same_v<Entity, sketch::SketchCircle>)
                    return 2.0 * std::numbers::pi_v<math::Scalar>;
                else return typed.geometry.sweepAngle();
            }();
            const auto start = [&]
            {
                if constexpr (std::is_same_v<Entity, sketch::SketchCircle>) return 0.0;
                else return typed.geometry.startAngle();
            }();
            const auto count = std::max<std::size_t>(2,
                static_cast<std::size_t>(std::ceil(circleSegments * std::abs(sweep)
                    / (2.0 * std::numbers::pi_v<math::Scalar>))));
            std::vector<geometry::Point3> points;
            points.reserve(count + 1);
            for (std::size_t i = 0; i <= count; ++i)
            {
                const auto t = static_cast<math::Scalar>(i) / count;
                if constexpr (std::is_same_v<Entity, sketch::SketchCircle>)
                    points.push_back(plane.toWorld(typed.geometry.pointAt(start + t * sweep)));
                else
                    points.push_back(plane.toWorld(typed.geometry.pointAt(t)));
            }
            return Polyline3{std::move(points)};
        }
    }, entity.geometry());
}
}

void SketchPresentation::regenerate(const sketch::Sketch& sketch)
{
    for (auto it = mapping_.begin(); it != mapping_.end();)
    {
        if (!sketch.contains(sketch::SketchEntityId{it->first}))
        {
            (void)geometry_.remove(it->second);
            it = mapping_.erase(it);
        }
        else ++it;
    }
    for (const auto id : sketch.entityIds())
    {
        auto derived = derive(sketch.find(id), sketch.plane());
        const auto mapped = mapping_.find(id.value());
        if (mapped == mapping_.end())
        {
            const auto visual = std::visit([this](const auto& value) { return geometry_.add(value); }, derived);
            mapping_.emplace(id.value(), visual);
        }
        else
            (void)geometry_.setGeometry(mapped->second, std::move(derived));
    }
}

std::optional<VisualEntityId> SketchPresentation::visualId(sketch::SketchEntityId id) const noexcept
{
    const auto found = mapping_.find(id.value());
    return found == mapping_.end() ? std::nullopt : std::optional{found->second};
}

std::optional<sketch::SketchEntityId> SketchPresentation::sketchId(VisualEntityId id) const noexcept
{
    for (const auto& [sketchValue, visual] : mapping_)
        if (visual == id) return sketch::SketchEntityId{sketchValue};
    return std::nullopt;
}

}
