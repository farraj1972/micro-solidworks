#pragma once

#include "core/math/Scalar.h"
#include "core/math/Vector3.h"
#include "viewer/VisualState.h"

#include <vector>

namespace microsw::presentation { class GeometryPresentation; }

namespace microsw::viewer
{
struct LinePresentationContext
{
    math::Vector3 viewCenter;
    math::Scalar visibleScale;
};

// Each Line3 becomes two endpoints centered on the support point nearest the
// view center. The half-length is three times the positive visible scale.
[[nodiscard]] std::vector<math::Vector3> presentedLineVertices(
    const presentation::GeometryPresentation& presentation,
    const LinePresentationContext& context,
    std::optional<VisualStateFilter> filter = std::nullopt);
}
