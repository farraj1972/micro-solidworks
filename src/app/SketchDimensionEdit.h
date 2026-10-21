#pragma once

#include "app/PersistenceSession.h"

namespace microsw
{

// Commits one Sketch-owned driving value only after the complete dependent
// FeatureModel rebuild succeeds. Failed candidates leave history/model intact.
[[nodiscard]] features::RebuildResult commitSketchDrivingDimension(
    PersistenceSession& session, features::FeatureId sketchFeature,
    const sketch::Sketch& currentSolvedSketch,
    sketch::SketchConstraintId dimension, math::Scalar value);

}
