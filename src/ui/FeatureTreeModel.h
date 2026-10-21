#pragma once

#include "features/FeatureModel.h"
#include "interaction/EditorContext.h"

#include <string>
#include <optional>
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

struct TopologySelectionDetails
{
    features::FeatureId owner;
    std::string geometryType;
    bool isFace{};
    std::optional<features::ExtrudeCapRole> faceRole;
};

[[nodiscard]] FeatureTreeModel deriveFeatureTree(
    const features::FeatureModel& model,
    const interaction::EditorContext* editor = nullptr);
[[nodiscard]] std::string featureStatusDescription(
    const features::FeatureModel& model, features::FeatureId id);
[[nodiscard]] std::optional<TopologySelectionDetails> inspectTopologySelection(
    const features::FeatureModel& model, const interaction::LogicalTarget& selection);
}
