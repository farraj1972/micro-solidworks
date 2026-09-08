#pragma once

#include "presentation/VisualEntity.h"

namespace microsw::presentation
{
// Derives canonical world Geometry without changing local Geometry or identity.
// Non-representable world coordinates/directions throw std::overflow_error.
[[nodiscard]] PresentedGeometry worldGeometry(const VisualEntity& entity);
}
