#include "presentation/FeaturePresentation.h"
#include <stdexcept>
namespace microsw::presentation {
void FeaturePresentation::update(const features::FeatureModel& model) {
 const auto resultId=model.resultFeatureId();
 if(!resultId.isValid()){current_={};return;}
 const auto& record=model.find(resultId);
 if(record.state()!=features::FeatureState::Clean||!record.cache()) throw std::invalid_argument{"Feature presentation requires a committed clean result"};
 if(std::holds_alternative<features::EmptyFeatureCache>(*record.cache())){current_={};return;}
 const auto* solid=std::get_if<features::SolidFeatureCache>(&*record.cache());
 if(!solid) throw std::invalid_argument{"Result feature does not contain a Solid output"};
 auto& presentation=presentations_[resultId]; presentation.regenerate(solid->solid); current_=presentation.geometry();
}
std::optional<VisualEntityId> FeaturePresentation::visualId(features::FeatureId id) const noexcept {
 const auto found=presentations_.find(id); return found==presentations_.end()?std::nullopt:found->second.visualId();
}
}
