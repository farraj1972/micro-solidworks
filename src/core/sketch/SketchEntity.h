#pragma once

#include "core/geometry/Arc2.h"
#include "core/geometry/Circle2.h"
#include "core/geometry/Segment2.h"
#include "core/sketch/SketchEntityId.h"

#include <variant>

namespace microsw::sketch
{

struct SketchLine { geometry::Segment2 geometry; };
struct SketchCircle { geometry::Circle2 geometry; };
struct SketchArc { geometry::Arc2 geometry; };
using SketchGeometry = std::variant<SketchLine, SketchCircle, SketchArc>;

enum class SketchEntityType { Line, Circle, Arc };

class SketchEntity
{
public:
    SketchEntity(SketchEntityId id, SketchGeometry geometry)
        : id_{id}, geometry_{std::move(geometry)} {}

    [[nodiscard]] SketchEntityId id() const noexcept { return id_; }
    [[nodiscard]] SketchEntityType type() const noexcept;
    [[nodiscard]] const SketchGeometry& geometry() const noexcept { return geometry_; }

private:
    SketchEntityId id_;
    SketchGeometry geometry_;
};

}
