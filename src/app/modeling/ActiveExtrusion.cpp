#include "app/modeling/ActiveExtrusion.h"

#include "app/modeling/SketchProfileAdapter.h"
#include "modeling/Extrusion.h"

#include <utility>

namespace microsw
{
void ActiveExtrusion::regenerate(const sketch::Sketch& sketch, math::Scalar distance)
{
    auto candidate = modeling::extrude(extractProfile(sketch), distance);
    presentation_.regenerate(candidate);
    solid_ = std::move(candidate);
}

const topology::Solid* ActiveExtrusion::solid() const noexcept
{
    return solid_ ? &*solid_ : nullptr;
}
}
