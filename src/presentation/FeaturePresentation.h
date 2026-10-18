#pragma once
#include "features/FeatureModel.h"
#include "presentation/SolidPresentation.h"
#include "presentation/SolidDisplay.h"
#include <functional>
#include <map>
#include <optional>
namespace microsw::presentation {
class FeaturePresentation {
public:
 using Visibility = std::function<bool(features::FeatureId)>;
 void update(const features::FeatureModel& model, const Visibility& visible = {});
 [[nodiscard]] const GeometryPresentation& geometry() const noexcept { return current_; }
 [[nodiscard]] std::optional<VisualEntityId> visualId(features::FeatureId id) const noexcept;
 [[nodiscard]] std::optional<features::FeatureId> featureId(VisualEntityId id) const noexcept;
 [[nodiscard]] const SolidDisplayData& solidDisplay() const noexcept { return solidDisplay_; }
 [[nodiscard]] const GeometryPresentation& sketchReferences() const noexcept { return sketchReferences_; }
private:
 std::map<features::FeatureId,std::vector<VisualEntityId>> visualIds_;
 GeometryPresentation current_;
 GeometryPresentation sketchReferences_;
 SolidDisplayData solidDisplay_;
};
}
