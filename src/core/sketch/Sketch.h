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

    void replaceLine(SketchEntityId id, const geometry::Segment2& geometry);
    void replaceCircle(SketchEntityId id, const geometry::Circle2& geometry);
    void replaceArc(SketchEntityId id, const geometry::Arc2& geometry);
    void remove(SketchEntityId id);
    [[nodiscard]] bool contains(SketchEntityId id) const noexcept;

    [[nodiscard]] const SketchEntity& find(SketchEntityId id) const;
    [[nodiscard]] std::vector<SketchEntityId> entityIds() const;
    [[nodiscard]] std::size_t size() const noexcept { return activeCount_; }

private:
    SketchEntityId add(SketchGeometry geometry);
    void replace(SketchEntityId id, SketchEntityType expected, SketchGeometry geometry);

    SketchPlane plane_;
    std::vector<std::optional<SketchEntity>> entities_;
    std::size_t activeCount_{};
};

}
