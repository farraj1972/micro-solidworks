#include "app/modeling/SketchProfileAdapter.h"

#include "features/SketchProfileAdapter.h"

namespace microsw
{
modeling::Profile extractProfile(const sketch::Sketch& sketch)
{
    return features::extractProfile(sketch);
}
}
