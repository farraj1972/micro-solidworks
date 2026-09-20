#include "app/modeling/SketchProfileAdapter.h"

#include "core/geometry/GeometricTolerance.h"

#include <algorithm>
#include <stdexcept>

namespace microsw
{
modeling::Profile extractProfile(const sketch::Sketch& sketch)
{
    struct Edge { geometry::Point2 a; geometry::Point2 b; };
    std::vector<Edge> remaining;
    for (const auto id : sketch.entityIds())
    {
        const auto& entity = sketch.find(id);
        if (entity.type() != sketch::SketchEntityType::Line)
            throw std::invalid_argument{"Profile extraction supports SketchLine entities only"};
        const auto& line = std::get<sketch::SketchLine>(entity.geometry()).geometry;
        if (line.isDegenerate()) throw std::invalid_argument{"Profile contains a degenerate line"};
        remaining.push_back({line.a(), line.b()});
    }
    if (remaining.size() < 3) throw std::invalid_argument{"Profile requires at least three SketchLines"};

    std::vector<geometry::Point2> ordered{remaining.front().a, remaining.front().b};
    remaining.erase(remaining.begin());
    while (!remaining.empty())
    {
        const auto current = ordered.back();
        std::size_t match = remaining.size();
        bool reverse = false;
        for (std::size_t i = 0; i < remaining.size(); ++i)
        {
            const bool atA = geometry::areCoincident(current, remaining[i].a);
            const bool atB = geometry::areCoincident(current, remaining[i].b);
            if (atA || atB)
            {
                if (match != remaining.size())
                    throw std::invalid_argument{"Profile connectivity is branching or ambiguous"};
                match = i; reverse = atB;
            }
        }
        if (match == remaining.size()) throw std::invalid_argument{"Profile contains an open or disconnected chain"};
        ordered.push_back(reverse ? remaining[match].a : remaining[match].b);
        remaining.erase(remaining.begin() + static_cast<std::ptrdiff_t>(match));
    }
    if (!geometry::areCoincident(ordered.back(), ordered.front()))
        throw std::invalid_argument{"Profile loop is open"};
    ordered.pop_back();

    math::Scalar twiceArea{};
    for (std::size_t i = 0; i < ordered.size(); ++i)
    {
        const auto& a = ordered[i]; const auto& b = ordered[(i + 1) % ordered.size()];
        twiceArea += a.x() * b.y() - b.x() * a.y();
    }
    if (twiceArea < 0) std::reverse(ordered.begin(), ordered.end());

    std::vector<geometry::Point3> world;
    world.reserve(ordered.size());
    for (const auto& point : ordered) world.push_back(sketch.plane().toWorld(point));
    return {std::move(world), {sketch.plane().origin(), sketch.plane().normal()}};
}
}
