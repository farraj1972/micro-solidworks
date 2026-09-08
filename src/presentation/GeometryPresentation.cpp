#include "presentation/GeometryPresentation.h"
#include "presentation/WorldGeometry.h"

#include <algorithm>
#include <utility>

namespace microsw::presentation
{
VisualEntityId GeometryPresentation::add(const geometry::Point3& point) { return add(PresentedGeometry{point}); }
VisualEntityId GeometryPresentation::add(const geometry::Segment3& segment) { return add(PresentedGeometry{segment}); }
VisualEntityId GeometryPresentation::add(const geometry::Line3& line) { return add(PresentedGeometry{line}); }

const VisualEntity* GeometryPresentation::find(VisualEntityId id) const noexcept
{
    const auto entity = std::find_if(entities_.begin(), entities_.end(),
        [id](const VisualEntity& candidate) { return candidate.id() == id; });
    return entity == entities_.end() ? nullptr : &*entity;
}

bool GeometryPresentation::setTransform(VisualEntityId id, const math::Transform3& transform)
{
    const auto entity = std::find_if(entities_.begin(), entities_.end(),
        [id](const VisualEntity& candidate) { return candidate.id() == id; });
    if (entity == entities_.end()) return false;
    auto candidate = *entity;
    candidate.setTransform(transform);
    (void)worldGeometry(candidate);
    entity->setTransform(transform);
    return true;
}

VisualEntityId GeometryPresentation::add(PresentedGeometry geometry)
{
    const VisualEntityId id = idGenerator_.generate();
    entities_.emplace_back(id, std::move(geometry));
    return id;
}
}
