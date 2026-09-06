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
private:
    [[nodiscard]] VisualEntityId add(PresentedGeometry geometry);
    VisualEntityIdGenerator idGenerator_;
    std::vector<VisualEntity> entities_;
};
}
