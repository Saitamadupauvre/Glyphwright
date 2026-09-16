#include "gw/editor/model/EntityListModel.hpp"
#include <algorithm>

namespace gw::editor {

std::vector<EntityListEntry> buildEntityList(const World& world) {
    std::vector<Entity> entities = world.allEntities();
    std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) { return a.id < b.id; });

    std::vector<EntityListEntry> result;
    result.reserve(entities.size() + 1);
    result.push_back(EntityListEntry{kInvalidEntity, "World", true});
    for (Entity e : entities) {
        result.push_back(EntityListEntry{e, "Entity " + std::to_string(e.id), false});
    }
    return result;
}

} // namespace gw::editor
