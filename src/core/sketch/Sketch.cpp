#include "core/sketch/Sketch.h"

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

const SketchEntity& Sketch::find(SketchEntityId id) const
{
    if (!id.isValid() || id.value() >= entities_.size() || !entities_[id.value()])
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
