#include "gw/editor/panels/EntityListPanel.hpp"
#include "gw/editor/model/EntityListModel.hpp"

namespace gw::editor {

EntityListPanel::EntityListPanel(const World& world) : _world(world) {}

PanelView EntityListPanel::view() const {
    PanelView view;
    view.title = "Entities";
    for (const auto& entry : buildEntityList(_world)) {
        view.rows.push_back(PanelRow{entry.label, RowStyle::Normal, 0, {}});
    }
    return view;
}

Entity EntityListPanel::entityAt(std::size_t index) const {
    return buildEntityList(_world).at(index).entity;
}

std::size_t EntityListPanel::entityCount() const {
    return buildEntityList(_world).size();
}

} // namespace gw::editor
