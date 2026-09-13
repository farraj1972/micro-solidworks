#include "core/sketch/SketchEntity.h"

namespace microsw::sketch
{

SketchEntityType SketchEntity::type() const noexcept
{
    if (std::holds_alternative<SketchLine>(geometry_)) return SketchEntityType::Line;
    if (std::holds_alternative<SketchCircle>(geometry_)) return SketchEntityType::Circle;
    return SketchEntityType::Arc;
}

}
