#pragma once
#include "features/FeatureModel.h"
#include "presentation/SolidPresentation.h"
#include <optional>
namespace microsw::presentation {
class FeaturePresentation {
public:
 void update(const features::FeatureModel& model);
 [[nodiscard]] const GeometryPresentation& geometry() const noexcept { return current_; }
 [[nodiscard]] std::optional<VisualEntityId> visualId(features::FeatureId id) const noexcept;
 [[nodiscard]] std::optional<features::FeatureId> featureId(VisualEntityId id) const noexcept;
private:
 std::optional<features::FeatureId> activeFeature_;
 std::vector<VisualEntityId> visualIds_;
 GeometryPresentation current_;
};
}
