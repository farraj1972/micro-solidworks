#include "core/sketch/Sketch.h"

#include "core/geometry/GeometricTolerance.h"

#include <cmath>
#include <stdexcept>

namespace microsw::sketch
{

SketchEntityId Sketch::add(SketchGeometry geometry)
{
    const auto id = SketchEntityId{static_cast<SketchEntityId::Value>(entities_.size())};
    if (!id.isValid()) throw std::overflow_error{"Sketch entity ID space exhausted"};
    entities_.emplace_back(std::in_place, id, std::move(geometry));
    ++activeCount_;
    return id;
}

SketchEntityId Sketch::addLine(const geometry::Segment2& geometry)
{
    return add(SketchLine{geometry});
}

SketchEntityId Sketch::addCircle(const geometry::Circle2& geometry)
{
    return add(SketchCircle{geometry});
}

SketchEntityId Sketch::addArc(const geometry::Arc2& geometry)
{
    return add(SketchArc{geometry});
}

std::array<SketchEntityId, 4> Sketch::addRectangle(
    const geometry::Point2& firstCorner,
    const geometry::Point2& oppositeCorner)
{
    if (std::abs(oppositeCorner.x() - firstCorner.x()) <= geometry::defaultGeometricTolerance
        || std::abs(oppositeCorner.y() - firstCorner.y()) <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Rectangle width and height must exceed geometric tolerance"};

    const geometry::Point2 second{oppositeCorner.x(), firstCorner.y()};
    const geometry::Point2 fourth{firstCorner.x(), oppositeCorner.y()};
    const auto originalSize = entities_.size();
    const auto originalActiveCount = activeCount_;
    if (originalSize > SketchEntityId::invalidValue - 4)
        throw std::overflow_error{"Sketch entity ID space exhausted"};
    entities_.reserve(originalSize + 4);
    try
    {
        return {addLine({firstCorner, second}), addLine({second, oppositeCorner}),
                addLine({oppositeCorner, fourth}), addLine({fourth, firstCorner})};
    }
    catch (...)
    {
        entities_.resize(originalSize);
        activeCount_ = originalActiveCount;
        throw;
    }
}

void Sketch::replace(SketchEntityId id, SketchEntityType expected, SketchGeometry geometry)
{
    const auto& current = find(id);
    if (current.type() != expected)
        throw std::invalid_argument{"Sketch entity replacement must preserve entity type"};
    entities_[id.value()].emplace(id, std::move(geometry));
}

void Sketch::replaceLine(SketchEntityId id, const geometry::Segment2& geometry)
{
    replace(id, SketchEntityType::Line, SketchLine{geometry});
}

void Sketch::replaceCircle(SketchEntityId id, const geometry::Circle2& geometry)
{
    replace(id, SketchEntityType::Circle, SketchCircle{geometry});
}

void Sketch::replaceArc(SketchEntityId id, const geometry::Arc2& geometry)
{
    replace(id, SketchEntityType::Arc, SketchArc{geometry});
}

void Sketch::remove(SketchEntityId id)
{
    (void)find(id);
    entities_[id.value()].reset();
    --activeCount_;
}

bool Sketch::contains(SketchEntityId id) const noexcept
{
    return id.isValid() && id.value() < entities_.size() && entities_[id.value()].has_value();
}

const SketchEntity& Sketch::find(SketchEntityId id) const
{
    if (!contains(id))
        throw std::out_of_range{"Invalid or removed SketchEntityId"};
    return *entities_[id.value()];
}

std::vector<SketchEntityId> Sketch::entityIds() const
{
    std::vector<SketchEntityId> result;
    result.reserve(activeCount_);
    for (const auto& entity : entities_)
        if (entity) result.push_back(entity->id());
    return result;
}

}
