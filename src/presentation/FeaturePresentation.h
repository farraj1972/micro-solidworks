#pragma once
#include "features/FeatureModel.h"
#include "presentation/SolidPresentation.h"
#include <map>
#include <optional>
namespace microsw::presentation {
class FeaturePresentation {
public:
 void update(const features::FeatureModel& model);
 [[nodiscard]] const GeometryPresentation& geometry() const noexcept { return current_; }
 [[nodiscard]] std::optional<VisualEntityId> visualId(features::FeatureId id) const noexcept;
private:
 std::map<features::FeatureId,SolidPresentation> presentations_;
 GeometryPresentation current_;
};
}
