#pragma once
#include <cstddef>
#include "gw/Entity.hpp"
#include "gw/World.hpp"
#include "gw/editor/PanelView.hpp"

namespace gw::editor {

class EntityListPanel {
public:
    explicit EntityListPanel(const World& world);

    PanelView view() const;
    Entity entityAt(std::size_t index) const;
    std::size_t entityCount() const;

private:
    const World& _world;
};

} // namespace gw::editor
