#pragma once
#include "modeling/Boolean.h"
#include "presentation/SolidPresentation.h"
#include <optional>
namespace microsw {
class ActiveBoolean {
public:
 void setOperandA(const topology::Solid& solid){operandA_=solid;}
 void setOperandB(const topology::Solid& solid){operandB_=solid;}
 [[nodiscard]] bool hasOperandA()const noexcept{return operandA_.has_value();}
 [[nodiscard]] bool hasOperandB()const noexcept{return operandB_.has_value();}
 [[nodiscard]] modeling::BooleanStatus execute(modeling::BooleanOperation operation);
 [[nodiscard]] modeling::BooleanStatus status()const noexcept{return status_;}
 [[nodiscard]] const topology::Solid* solid()const noexcept{return result_?&*result_:nullptr;}
 [[nodiscard]] const presentation::SolidPresentation& presentation()const noexcept{return presentation_;}
private:
 std::optional<topology::Solid> operandA_,operandB_,result_;
 presentation::SolidPresentation presentation_;
 modeling::BooleanStatus status_{modeling::BooleanStatus::INVALID_INPUT};
};
}
