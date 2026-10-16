#include "app/PersistenceSession.h"
namespace microsw {
void PersistenceSession::newDocument(){model_=features::FeatureModel{};currentFilePath_.reset();}
persistence::SaveResult PersistenceSession::save()noexcept{if(!currentFilePath_)return{persistence::SaveStatus::InvalidPath,"Save As is required"};return persistence::saveFeatureModel(model_,*currentFilePath_);}
persistence::SaveResult PersistenceSession::saveAs(const std::filesystem::path&p)noexcept{auto r=persistence::saveFeatureModel(model_,p);if(r.status==persistence::SaveStatus::Success)currentFilePath_=p;return r;}
persistence::OpenStatus PersistenceSession::open(const std::filesystem::path&p,std::string*d)noexcept{auto r=persistence::openFeatureModel(p);if(d)*d=r.diagnostic;if(r.status==persistence::OpenStatus::SuccessRebuilt||r.status==persistence::OpenStatus::SuccessRebuildFailed){model_=std::move(r.model);currentFilePath_=p;}return r.status;}
}
