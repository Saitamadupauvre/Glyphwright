#include "gw/World.hpp"

namespace gw {

Entity World::createEntity() {
    return Entity{_nextEntityId++, 0};
}

void World::destroyEntity(Entity e) {
    for (auto& [type, pool] : _pools) {
        pool.remove(e);
    }
}

} // namespace gw
