#pragma once

#include "core/sketch/Sketch.h"
#include "modeling/Profile.h"

namespace microsw::features
{
[[nodiscard]] modeling::Profile extractProfile(const sketch::Sketch& sketch);
}
