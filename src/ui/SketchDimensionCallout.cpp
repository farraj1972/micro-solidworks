#include "ui/SketchDimensionCallout.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <type_traits>

namespace microsw::ui
{
namespace
{
std::optional<geometry::Point2> pointFor(const sketch::Sketch& sketch,
    sketch::SketchElementRef ref)
{
    if (!sketch.contains(ref.entity)) return std::nullopt;
    const auto& geometry = sketch.find(ref.entity).geometry();
    if (const auto* line = std::get_if<sketch::SketchLine>(&geometry))
    {
        if (ref.kind == sketch::SubElementKind::LineStart) return line->geometry.a();
        if (ref.kind == sketch::SubElementKind::LineEnd) return line->geometry.b();
        if (ref.kind == sketch::SubElementKind::LineBody) return line->geometry.midpoint();
    }
    if (const auto* circle = std::get_if<sketch::SketchCircle>(&geometry))
    {
        if (ref.kind == sketch::SubElementKind::CircleCenter) return circle->geometry.center();
        if (ref.kind == sketch::SubElementKind::CircleRadius)
            return circle->geometry.pointAt(0.0);
    }
    return std::nullopt;
}

std::string millimetres(const char* prefix, math::Scalar value)
{
    std::ostringstream out;
    out << prefix << std::setprecision(6) << value << " mm";
    return out.str();
}
}

std::vector<SketchDimensionCallout> deriveSketchDimensionCallouts(
    const sketch::Sketch& solvedSketch, const SketchPointProjector& project)
{
    std::vector<SketchDimensionCallout> result;
    for (const auto id : solvedSketch.constraintIds())
    {
        const auto& value = solvedSketch.findConstraint(id).value();
        if (!sketch::isDrivingDimension(value)) continue;
        std::optional<geometry::Point2> first, second;
        sketch::SketchEntityId entity;
        std::string text;
        enum class Kind { Length, Horizontal, Vertical, Radius } kind{};
        std::visit([&](const auto& dimension) {
            using T = std::decay_t<decltype(dimension)>;
            if constexpr (std::is_same_v<T, sketch::LineLength>)
            {
                entity = dimension.line.entity;
                if (solvedSketch.contains(entity))
                    if (const auto* line = std::get_if<sketch::SketchLine>(
                            &solvedSketch.find(entity).geometry()))
                { first = line->geometry.a(); second = line->geometry.b(); }
                text = millimetres("L ", dimension.value);
                kind = Kind::Length;
            }
            else if constexpr (std::is_same_v<T, sketch::CircleRadius>)
            {
                entity = dimension.radius.entity;
                if (solvedSketch.contains(entity))
                    if (const auto* circle = std::get_if<sketch::SketchCircle>(
                            &solvedSketch.find(entity).geometry()))
                { first = circle->geometry.center(); second = circle->geometry.pointAt(0.0); }
                text = millimetres("R ", dimension.value);
                kind = Kind::Radius;
            }
            else if constexpr (std::is_same_v<T, sketch::HorizontalDistance>
                || std::is_same_v<T, sketch::VerticalDistance>)
            {
                entity = dimension.first.entity;
                first = pointFor(solvedSketch, dimension.first);
                second = pointFor(solvedSketch, dimension.second);
                text = millimetres(std::is_same_v<T, sketch::HorizontalDistance> ? "X " : "Y ",
                    dimension.value);
                kind = std::is_same_v<T, sketch::HorizontalDistance>
                    ? Kind::Horizontal : Kind::Vertical;
            }
        }, value);
        if (!first || !second) continue;
        const auto a = project(*first), b = project(*second);
        if (!a || !b) continue;
        const double dx = b->x - a->x, dy = b->y - a->y;
        interaction::ScreenPoint label{(a->x + b->x) / 2.0, (a->y + b->y) / 2.0};
        if (kind == Kind::Vertical) label.x = std::max(a->x, b->x) + 26.0;
        else if (kind == Kind::Horizontal) label.y = std::min(a->y, b->y) - 26.0;
        else if (kind == Kind::Radius) { label.x = b->x + 30.0; label.y = b->y - 24.0; }
        else
        {
            const auto length = std::hypot(dx, dy);
            if (length > 1.0) { label.x += 22.0 * dy / length; label.y -= 22.0 * dx / length; }
            else label.y -= 22.0;
        }
        // Stable, minimal staggering avoids hiding a newly created dimension.
        for (const auto& existing : result)
            if (std::abs(existing.label.x - label.x) < 80.0
                && std::abs(existing.label.y - label.y) < 24.0)
                label.y = existing.label.y + 26.0;
        result.push_back({id, entity, *a, *b, label, std::move(text)});
    }
    return result;
}
}
