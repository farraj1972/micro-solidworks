#include "app/PersistenceSession.h"
namespace microsw {
PersistenceSession::PersistenceSession():model_{},history_{model_}{}
PersistenceSession::PersistenceSession(features::FeatureModel model):model_{std::move(model)},history_{model_}{}
void PersistenceSession::newDocument(){model_=features::FeatureModel{};currentFilePath_.reset();history_.reset(model_,true);}
persistence::SaveResult PersistenceSession::save()noexcept{if(!currentFilePath_)return{persistence::SaveStatus::InvalidPath,"Save As is required"};auto r=persistence::saveFeatureModel(model_,*currentFilePath_);if(r.status==persistence::SaveStatus::Success)history_.markSaved(model_);return r;}
persistence::SaveResult PersistenceSession::saveAs(const std::filesystem::path&p)noexcept{auto r=persistence::saveFeatureModel(model_,p);if(r.status==persistence::SaveStatus::Success){currentFilePath_=p;history_.markSaved(model_);}return r;}
persistence::OpenStatus PersistenceSession::open(const std::filesystem::path&p,std::string*d)noexcept{auto r=persistence::openFeatureModel(p);if(d)*d=r.diagnostic;if(r.status==persistence::OpenStatus::SuccessRebuilt||r.status==persistence::OpenStatus::SuccessRebuildFailed){model_=std::move(r.model);currentFilePath_=p;history_.reset(model_,true);}return r.status;}
bool PersistenceSession::undo(){if(!history_.undo(model_))return false;(void)model_.rebuild();return true;}
bool PersistenceSession::redo(){if(!history_.redo(model_))return false;(void)model_.rebuild();return true;}
}
