#include "presentation/FeaturePresentation.h"
#include <algorithm>
#include <stdexcept>
namespace microsw::presentation {
void FeaturePresentation::update(const features::FeatureModel& model) {
 const auto resultId=model.resultFeatureId();
 if(!resultId.isValid()){current_={};activeFeature_.reset();visualIds_.clear();return;}
 const auto& record=model.find(resultId);
 if(record.state()!=features::FeatureState::Clean||!record.cache()) throw std::invalid_argument{"Feature presentation requires a committed clean result"};
 if(std::holds_alternative<features::EmptyFeatureCache>(*record.cache())){current_={};activeFeature_=resultId;visualIds_.clear();return;}
 const auto* solid=std::get_if<features::SolidFeatureCache>(&*record.cache());
 if(!solid) throw std::invalid_argument{"Result feature does not contain a Solid output"};
 if(activeFeature_!=resultId){current_={};visualIds_.clear();activeFeature_=resultId;}
 while(visualIds_.size()>solid->solids.size()){current_.remove(visualIds_.back());visualIds_.pop_back();}
 for(std::size_t index=0;index<solid->solids.size();++index){
  auto wireframe=deriveSolidWireframe(solid->solids[index]);
  if(index<visualIds_.size())current_.setGeometry(visualIds_[index],std::move(wireframe));
  else visualIds_.push_back(current_.add(wireframe));
 }
}
std::optional<VisualEntityId> FeaturePresentation::visualId(features::FeatureId id) const noexcept {
 return activeFeature_==id&&!visualIds_.empty()?std::optional{visualIds_.front()}:std::nullopt;
}
std::optional<features::FeatureId> FeaturePresentation::featureId(VisualEntityId id) const noexcept {
 return activeFeature_&&std::find(visualIds_.begin(),visualIds_.end(),id)!=visualIds_.end()?activeFeature_:std::nullopt;
}
}
