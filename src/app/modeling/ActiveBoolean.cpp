#include "app/modeling/ActiveBoolean.h"
namespace microsw {
modeling::BooleanStatus ActiveBoolean::execute(modeling::BooleanOperation operation){
 if(!operandA_||!operandB_){status_=modeling::BooleanStatus::INVALID_INPUT;return status_;}
 auto candidate=modeling::booleanOperation(*operandA_,*operandB_,operation);status_=candidate.status();
 if(status_==modeling::BooleanStatus::SUCCESS){auto solid=*candidate.solid();presentation_.regenerate(solid);result_=std::move(solid);}
 else if(status_==modeling::BooleanStatus::EMPTY)result_.reset();
 return status_;
}
}
