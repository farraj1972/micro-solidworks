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
        if (record.kind() != features::FeatureKind::Extrude) continue;
        const auto sketch = std::get<features::ExtrudeFeature>(record.data()).sourceSketchFeatureId;
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
}
