#pragma once
#include "presentation/SolidDisplay.h"
#include "viewer/PickingContext.h"
#include <optional>
#include <variant>
namespace microsw::viewer {
using SolidTarget=std::variant<interaction::TopologyFaceTarget,interaction::TopologyEdgeTarget>;
struct SolidPickHit { SolidTarget target; math::Scalar distance; };
[[nodiscard]] std::optional<SolidPickHit> pickSolid(const presentation::SolidDisplayData& display,
 const PickingContext& context, math::Scalar edgeTolerancePixels=6.0);
}
