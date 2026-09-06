#pragma once

#include "presentation/GeometryPresentation.h"

namespace microsw::demo
{
// Temporary B4 composition, returned with value ownership to the application.
// IDs belong to each collection's generator, not a persistent/global identity.
[[nodiscard]] presentation::GeometryPresentation createGeometryDemoScene();
}
