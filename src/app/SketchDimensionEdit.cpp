#include "app/SketchDimensionEdit.h"

#include <stdexcept>
#include <utility>

namespace microsw
{

features::RebuildResult commitSketchDrivingDimension(
    PersistenceSession& session, features::FeatureId sketchFeature,
    const sketch::Sketch& currentSolvedSketch,
    sketch::SketchConstraintId dimension, math::Scalar value)
{
    if (!currentSolvedSketch.containsConstraint(dimension))
        throw std::invalid_argument{"The selected dimension is no longer available."};
    auto candidateSketch = currentSolvedSketch;
    candidateSketch.setDrivingValue(dimension, value);

    auto candidateModel = session.model();
    candidateModel.editSketch(sketchFeature, [&](auto& sketch) { sketch = candidateSketch; });
    const auto result = candidateModel.rebuild();
    if (result.status == features::RebuildStatus::Failed)
        throw std::runtime_error{result.diagnostic.empty()
            ? "Dimension cannot be solved; previous value preserved." : result.diagnostic};

    session.model() = std::move(candidateModel);
    session.recordEdit();
    return result;
}

}
