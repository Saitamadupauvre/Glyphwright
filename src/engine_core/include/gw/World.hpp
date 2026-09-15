#pragma once
#include <cstdint>
#include <cstring>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
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

    template <typename T>
    const std::vector<Entity>& each() const {
        static const std::vector<Entity> kEmpty;
        auto it = _pools.find(std::type_index(typeid(T)));
        if (it == _pools.end()) return kEmpty;
        return it->second.entities();
    }

    std::vector<std::type_index> componentTypesOf(Entity e) const {
        std::vector<std::type_index> types;
        for (const auto& [type, pool] : _pools) {
            if (pool.get(e) != nullptr) types.push_back(type);
        }
        return types;
    }

    std::vector<Entity> allEntities() const {
        std::vector<Entity> result;
        std::unordered_set<uint32_t> seen;
        for (const auto& [type, pool] : _pools) {
            for (Entity e : pool.entities()) {
                if (seen.insert(e.id).second) result.push_back(e);
            }
        }
        return result;
    }

    void* addComponentRaw(Entity e, std::type_index type, size_t size, size_t align) {
        auto it = _pools.find(type);
        if (it == _pools.end()) {
            it = _pools.emplace(type, ComponentPool(size, align)).first;
        }
        return it->second.emplace(e);
    }

    void* getComponentRaw(Entity e, std::type_index type) {
        auto it = _pools.find(type);
        if (it == _pools.end()) return nullptr;
        return it->second.get(e);
    }

    const void* getComponentRaw(Entity e, std::type_index type) const {
        auto it = _pools.find(type);
        if (it == _pools.end()) return nullptr;
        return it->second.get(e);
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
