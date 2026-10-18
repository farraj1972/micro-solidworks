#include "presentation/FeaturePresentation.h"
#include "presentation/SketchPresentation.h"
#include <algorithm>
#include <stdexcept>
namespace microsw::presentation {
void FeaturePresentation::update(const features::FeatureModel& model, const Visibility& visible) {
 solidDisplay_={};sketchReferences_={};std::vector<features::FeatureId> presented;
 for(const auto& record:model.history()){
  if(record.state()!=features::FeatureState::Clean||!record.cache())continue;
  if(record.kind()==features::FeatureKind::Sketch&&(!visible||visible(record.id()))){
   SketchPresentation sketch;sketch.regenerate(std::get<features::SketchFeatureCache>(*record.cache()).solvedSketch);
   for(const auto& entity:sketch.geometry().entities())std::visit([&](const auto& geometry){(void)sketchReferences_.add(geometry);},entity.geometry());
   continue;
  }
  const auto* solid=std::get_if<features::SolidFeatureCache>(&*record.cache());
  if(!solid||(visible&&!visible(record.id())))continue;
  presented.push_back(record.id());auto& ids=visualIds_[record.id()];
  while(ids.size()>solid->solids.size()){current_.remove(ids.back());ids.pop_back();}
  for(std::size_t index=0;index<solid->solids.size();++index){
   auto wireframe=deriveSolidWireframe(solid->solids[index]);
   if(index<ids.size())current_.setGeometry(ids[index],std::move(wireframe));
   else ids.push_back(current_.add(wireframe));
   auto display=deriveSolidDisplay(solid->solids[index],record.id(),interaction::BodyInstanceContext{index});
   solidDisplay_.triangles.insert(solidDisplay_.triangles.end(),display.triangles.begin(),display.triangles.end());
   solidDisplay_.edges.insert(solidDisplay_.edges.end(),display.edges.begin(),display.edges.end());
  }
 }
 for(auto it=visualIds_.begin();it!=visualIds_.end();){
  if(std::find(presented.begin(),presented.end(),it->first)!=presented.end()){++it;continue;}
  for(const auto id:it->second)current_.remove(id);it=visualIds_.erase(it);
 }
}
std::optional<VisualEntityId> FeaturePresentation::visualId(features::FeatureId id) const noexcept {
 const auto found=visualIds_.find(id);return found!=visualIds_.end()&&!found->second.empty()?std::optional{found->second.front()}:std::nullopt;
}
std::optional<features::FeatureId> FeaturePresentation::featureId(VisualEntityId id) const noexcept {
 for(const auto&[feature,ids]:visualIds_)if(std::find(ids.begin(),ids.end(),id)!=ids.end())return feature;return std::nullopt;
}
}
