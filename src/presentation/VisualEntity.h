#pragma once

#include "core/geometry/Line3.h"
#include "core/geometry/Point3.h"
#include "core/geometry/Segment3.h"
#include "presentation/VisualEntityId.h"

#include <utility>
#include <variant>

namespace microsw::presentation
{
using PresentedGeometry = std::variant<geometry::Point3, geometry::Segment3, geometry::Line3>;

class VisualEntity
{
public:
    VisualEntity(VisualEntityId id, PresentedGeometry geometry)
        : id_{id}, geometry_{std::move(geometry)} {}
    [[nodiscard]] VisualEntityId id() const noexcept { return id_; }
    [[nodiscard]] const PresentedGeometry& geometry() const noexcept { return geometry_; }
private:
    VisualEntityId id_;
    PresentedGeometry geometry_;
};
}
