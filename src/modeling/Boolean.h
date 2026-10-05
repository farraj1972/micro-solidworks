#pragma once
#include "core/topology/Solid.h"
#include <optional>
#include <utility>
namespace microsw::modeling {
enum class BooleanOperation { Union, Difference, Intersection };
enum class BooleanStatus { SUCCESS, EMPTY, INVALID_INPUT, UNSUPPORTED_CASE, NUMERICAL_FAILURE };
class BooleanResult {
public:
 [[nodiscard]] BooleanStatus status() const noexcept { return status_; }
 [[nodiscard]] const std::optional<topology::Solid>& solid() const noexcept { return solid_; }
 [[nodiscard]] static BooleanResult success(topology::Solid);
 [[nodiscard]] static BooleanResult failure(BooleanStatus);
private:
 BooleanResult(BooleanStatus s,std::optional<topology::Solid> v):status_{s},solid_{std::move(v)}{}
 BooleanStatus status_; std::optional<topology::Solid> solid_;
};
[[nodiscard]] BooleanResult booleanOperation(const topology::Solid&,const topology::Solid&,BooleanOperation) noexcept;
}
