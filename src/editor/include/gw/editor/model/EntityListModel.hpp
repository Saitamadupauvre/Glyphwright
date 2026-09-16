#pragma once
#include <string>
#include <vector>
#include "gw/Entity.hpp"
#include "gw/World.hpp"

namespace gw::editor {

struct EntityListEntry {
    Entity entity;
    std::string label;
    bool isWorld = false;
};

std::vector<EntityListEntry> buildEntityList(const World& world);

} // namespace gw::editor
