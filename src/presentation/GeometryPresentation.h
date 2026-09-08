#pragma once

#include "presentation/VisualEntity.h"

#include <cstddef>
#include <vector>

namespace microsw::presentation
{
class GeometryPresentation
{
public:
    [[nodiscard]] VisualEntityId add(const geometry::Point3& point);
    [[nodiscard]] VisualEntityId add(const geometry::Segment3& segment);
    [[nodiscard]] VisualEntityId add(const geometry::Line3& line);
    [[nodiscard]] const std::vector<VisualEntity>& entities() const noexcept { return entities_; }
    [[nodiscard]] std::size_t size() const noexcept { return entities_.size(); }
    [[nodiscard]] bool empty() const noexcept { return entities_.empty(); }
    [[nodiscard]] const VisualEntity* find(VisualEntityId id) const noexcept;
    // Unknown ID returns false. Validate derived Geometry before committing;
    // non-representable results throw, preserving the previous entity value.
    bool setTransform(VisualEntityId id, const math::Transform3& transform);
private:
    [[nodiscard]] VisualEntityId add(PresentedGeometry geometry);
    VisualEntityIdGenerator idGenerator_;
    std::vector<VisualEntity> entities_;
};
}
