#pragma once
#include <cstdint>
#include <cstring>
#include <typeindex>
#include <unordered_map>
#include "gw/ComponentPool.hpp"
#include "gw/Entity.hpp"

namespace gw {

class World {
public:
    Entity createEntity();
    void destroyEntity(Entity e);

    template <typename T>
    T& addComponent(Entity e, T value) {
        ComponentPool& pool = poolFor<T>();
        void* slot = pool.emplace(e);
        std::memcpy(slot, &value, sizeof(T));
        return *static_cast<T*>(slot);
    }

    template <typename T>
    T* getComponent(Entity e) {
        auto it = _pools.find(std::type_index(typeid(T)));
        if (it == _pools.end()) return nullptr;
        return static_cast<T*>(it->second.get(e));
    }

    template <typename T>
    void removeComponent(Entity e) {
        poolFor<T>().remove(e);
    }

private:
    template <typename T>
    ComponentPool& poolFor() {
        auto key = std::type_index(typeid(T));
        auto it = _pools.find(key);
        if (it == _pools.end()) {
            it = _pools.emplace(key, ComponentPool(sizeof(T), alignof(T))).first;
        }
        return it->second;
    }

    uint32_t _nextEntityId = 1;
    std::unordered_map<std::type_index, ComponentPool> _pools;
};

} // namespace gw
