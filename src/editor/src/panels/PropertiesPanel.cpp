#include "gw/editor/panels/PropertiesPanel.hpp"

namespace gw::editor {

PropertiesPanel::PropertiesPanel(const World& world, const ReflectionRegistry& registry,
                                  std::vector<KnownType> knownTypes)
    : _world(world), _registry(registry), _knownTypes(std::move(knownTypes)) {}

PanelView PropertiesPanel::view(std::optional<Entity> selected) const {
    PanelView view;
    view.title = "Properties";
    if (!selected.has_value()) return view;

    for (const auto& component : buildInspector(_world, *selected, _registry, _knownTypes)) {
        PanelRow row{component.typeName, RowStyle::Bold, 0, {}};
        for (const auto& field : component.fields) {
            row.children.push_back(PanelRow{field.name + " = " + field.valueText, RowStyle::Normal, 1, {}});
        }
        view.rows.push_back(std::move(row));
    }
    return view;
}

} // namespace gw::editor
