#pragma once

#include "features/FeatureModel.h"
#include "interaction/EditorContext.h"

#include <string>
#include <vector>

namespace microsw::ui
{
struct FeatureTreeNode
{
    features::FeatureId featureId;
    std::string displayName;
    features::FeatureKind kind;
    features::FeatureState state;
    interaction::DisplayState displayState;
    bool activeEditing{};
    std::vector<FeatureTreeNode> children;
};

struct FeatureTreeModel
{
    std::string rootName{"Part1"};
    std::vector<FeatureTreeNode> roots;
};

[[nodiscard]] FeatureTreeModel deriveFeatureTree(
    const features::FeatureModel& model,
    const interaction::EditorContext* editor = nullptr);
[[nodiscard]] std::string featureStatusDescription(
    const features::FeatureModel& model, features::FeatureId id);
}
