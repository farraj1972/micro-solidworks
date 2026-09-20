#pragma once

#include "core/topology/Solid.h"
#include "presentation/GeometryPresentation.h"

#include <optional>

namespace microsw::presentation
{
class SolidPresentation
{
public:
    void regenerate(const topology::Solid& solid);
    [[nodiscard]] const GeometryPresentation& geometry() const noexcept { return geometry_; }
    [[nodiscard]] std::optional<VisualEntityId> visualId() const noexcept { return visualId_; }
private:
    GeometryPresentation geometry_;
    std::optional<VisualEntityId> visualId_;
};
}
