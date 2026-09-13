#pragma once

#include "core/sketch/SketchEntity.h"
#include "core/sketch/SketchPlane.h"

#include <optional>
#include <vector>

namespace microsw::sketch
{

class Sketch
{
public:
    Sketch() = default;
    explicit Sketch(const SketchPlane& plane) : plane_{plane} {}

    [[nodiscard]] const SketchPlane& plane() const noexcept { return plane_; }

    SketchEntityId addLine(const geometry::Segment2& geometry);
    SketchEntityId addCircle(const geometry::Circle2& geometry);
    SketchEntityId addArc(const geometry::Arc2& geometry);

    [[nodiscard]] const SketchEntity& find(SketchEntityId id) const;
    [[nodiscard]] std::vector<SketchEntityId> entityIds() const;
    [[nodiscard]] std::size_t size() const noexcept { return activeCount_; }

private:
    SketchEntityId add(SketchGeometry geometry);

    SketchPlane plane_;
    std::vector<std::optional<SketchEntity>> entities_;
    std::size_t activeCount_{};
};

}
