#pragma once

#include "core/sketch/Sketch.h"
#include "presentation/GeometryPresentation.h"

#include <map>
#include <optional>

namespace microsw::presentation
{

class SketchPresentation
{
public:
    void regenerate(const sketch::Sketch& sketch);
    [[nodiscard]] const GeometryPresentation& geometry() const noexcept { return geometry_; }
    [[nodiscard]] std::optional<VisualEntityId> visualId(sketch::SketchEntityId id) const noexcept;
    [[nodiscard]] std::optional<sketch::SketchEntityId> sketchId(VisualEntityId id) const noexcept;

private:
    GeometryPresentation geometry_;
    std::map<sketch::SketchEntityId::Value, VisualEntityId> mapping_;
};

}
