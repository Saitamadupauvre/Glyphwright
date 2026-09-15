#pragma once
#include <optional>
#include <vector>
#include "gw/Entity.hpp"
#include "gw/Reflection.hpp"
#include "gw/World.hpp"
#include "gw/editor/model/InspectorModel.hpp"
#include "gw/editor/PanelView.hpp"

namespace gw::editor {

class PropertiesPanel {
public:
    PropertiesPanel(const World& world, const ReflectionRegistry& registry,
                     std::vector<KnownType> knownTypes, std::vector<KnownType> knownSingletonTypes = {});

    PanelView view(std::optional<Entity> selected) const;

private:
    const World& _world;
    const ReflectionRegistry& _registry;
    std::vector<KnownType> _knownTypes;
    std::vector<KnownType> _knownSingletonTypes;
};

} // namespace gw::editor
