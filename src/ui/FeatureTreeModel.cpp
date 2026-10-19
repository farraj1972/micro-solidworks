#include "ui/FeatureTreeModel.h"

#include <map>
#include <optional>

namespace microsw::ui
{
FeatureTreeModel deriveFeatureTree(const features::FeatureModel& model,
                                   const interaction::EditorContext* editor)
{
    std::map<features::FeatureId, features::FeatureId> primaryConsumer;
    for (const auto& record : model.history())
    {
        if (record.kind() != features::FeatureKind::Extrude
            && record.kind() != features::FeatureKind::CutExtrude) continue;
        const auto sketch = record.kind() == features::FeatureKind::Extrude
            ? std::get<features::ExtrudeFeature>(record.data()).sourceSketchFeatureId
            : std::get<features::CutExtrudeFeature>(record.data()).sourceSketchFeatureId;
        primaryConsumer.try_emplace(sketch, record.id());
    }

    const auto makeNode = [&](const features::FeatureRecord& record)
    {
        const auto display = editor ? editor->displayState(record.id())
                                    : interaction::DisplayState::Visible;
        return FeatureTreeNode{record.id(), record.displayName(), record.kind(),
            record.state(), display,
            editor && editor->activeSketchFeatureId() == record.id(), {}};
    };

    FeatureTreeModel tree;
    std::map<features::FeatureId, std::size_t> rootById;
    for (const auto& record : model.history())
    {
        if (record.kind() == features::FeatureKind::Sketch
            && primaryConsumer.contains(record.id())) continue;
        rootById.emplace(record.id(), tree.roots.size());
        tree.roots.push_back(makeNode(record));
    }

    for (const auto& record : model.history())
    {
        if (record.kind() != features::FeatureKind::Sketch) continue;
        const auto consumer = primaryConsumer.find(record.id());
        if (consumer == primaryConsumer.end()) continue;
        const auto parent = rootById.find(consumer->second);
        if (parent != rootById.end())
            tree.roots[parent->second].children.push_back(makeNode(record));
    }
    return tree;
}

std::string featureStatusDescription(const features::FeatureModel& model,
                                     features::FeatureId id)
{
    const auto& feature = model.find(id);
    switch (feature.state())
    {
    case features::FeatureState::Clean: return {};
    case features::FeatureState::Dirty: return "Needs rebuild";
    case features::FeatureState::Failed:
        return "Failed: " + (feature.diagnostic().empty()
            ? std::string{"Review this feature and rebuild."} : feature.diagnostic());
    case features::FeatureState::BlockedByDependency:
        for (const auto dependency : feature.dependencies())
        {
            const auto& upstream = model.find(dependency);
            if (upstream.state() == features::FeatureState::Failed)
                return "Blocked because " + upstream.displayName() + " failed. "
                    + featureStatusDescription(model, dependency);
            if (upstream.state() == features::FeatureState::BlockedByDependency)
                return "Blocked because " + upstream.displayName() + " is blocked. "
                    + featureStatusDescription(model, dependency);
        }
        return "Blocked by an earlier feature. Review upstream dependencies.";
    }
    return {};
}
}
