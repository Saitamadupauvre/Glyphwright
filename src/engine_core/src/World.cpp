#include "gw/World.hpp"

namespace gw {

Entity World::createEntity() {
    uint32_t id;
    if (!_freeIds.empty()) {
        id = _freeIds.back();
        _freeIds.pop_back();
    } else {
        id = _nextEntityId++;
        _generations.resize(static_cast<size_t>(id) + 1, 0);
        _alive.resize(static_cast<size_t>(id) + 1, 0);
    }
    _alive[id] = 1;
    ++_aliveCount;
    return Entity{id, _generations[id]};
}

bool World::isAlive(Entity e) const {
    return e.id != 0 && e.id < _alive.size() && _alive[e.id] != 0 && _generations[e.id] == e.generation;
}

void World::destroyEntity(Entity e) {
    if (!isAlive(e)) return;
    for (auto& [type, pool] : _pools) {
        pool.remove(e);
    }
    _alive[e.id] = 0;
    ++_generations[e.id];
    _freeIds.push_back(e.id);
    --_aliveCount;
}

} // namespace gw
