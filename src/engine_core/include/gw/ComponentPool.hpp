#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>
#include "gw/Entity.hpp"

namespace gw {

class ComponentPool {
public:
    ComponentPool(size_t element_size, size_t element_align)
        : _element_size(element_size), _element_align(element_align) {}

    ~ComponentPool() {
        if (_data) {
            std::free(_data);
        }
    }

    ComponentPool(const ComponentPool&) = delete;
    ComponentPool& operator=(const ComponentPool&) = delete;

    ComponentPool(ComponentPool&& other) noexcept
        : _element_size(other._element_size),
          _element_align(other._element_align),
          _data(other._data),
          _count(other._count),
          _capacity(other._capacity),
          _entity_of_index(std::move(other._entity_of_index)),
          _index_of_entity(std::move(other._index_of_entity)) {
        other._data = nullptr;
        other._count = 0;
        other._capacity = 0;
    }

    ComponentPool& operator=(ComponentPool&& other) noexcept {
        if (this == &other) return *this;
        if (_data) std::free(_data);
        _element_size = other._element_size;
        _element_align = other._element_align;
        _data = other._data;
        _count = other._count;
        _capacity = other._capacity;
        _entity_of_index = std::move(other._entity_of_index);
        _index_of_entity = std::move(other._index_of_entity);
        other._data = nullptr;
        other._count = 0;
        other._capacity = 0;
        return *this;
    }

    void* emplace(Entity e) {
        if (_count == _capacity) grow();
        size_t index = _count++;
        _entity_of_index.push_back(e);
        _index_of_entity[e.id] = index;
        return elementAt(index);
    }

    void remove(Entity e) {
        auto it = _index_of_entity.find(e.id);
        if (it == _index_of_entity.end()) return;
        size_t index = it->second;
        size_t last = _count - 1;
        if (index != last) {
            std::memcpy(elementAt(index), elementAt(last), _element_size);
            Entity moved = _entity_of_index[last];
            _entity_of_index[index] = moved;
            _index_of_entity[moved.id] = index;
        }
        _entity_of_index.pop_back();
        _index_of_entity.erase(it);
        --_count;
    }

    void* get(Entity e) {
        auto it = _index_of_entity.find(e.id);
        if (it == _index_of_entity.end()) return nullptr;
        return elementAt(it->second);
    }

    const void* get(Entity e) const {
        auto it = _index_of_entity.find(e.id);
        if (it == _index_of_entity.end()) return nullptr;
        return elementAt(it->second);
    }

    void* rawData() { return _data; }
    size_t size() const { return _count; }
    const std::vector<Entity>& entities() const { return _entity_of_index; }

private:
    void grow() {
        size_t new_capacity = _capacity == 0 ? 16 : _capacity * 2;
        void* new_data = std::aligned_alloc(_element_align, alignUp(new_capacity * _element_size, _element_align));
        if (_data) {
            std::memcpy(new_data, _data, _count * _element_size);
            std::free(_data);
        }
        _data = new_data;
        _capacity = new_capacity;
    }

    void* elementAt(size_t index) {
        return static_cast<std::byte*>(_data) + index * _element_size;
    }

    const void* elementAt(size_t index) const {
        return static_cast<const std::byte*>(_data) + index * _element_size;
    }

    static size_t alignUp(size_t n, size_t align) {
        return (n + align - 1) / align * align;
    }

    size_t _element_size;
    size_t _element_align;
    void* _data = nullptr;
    size_t _count = 0;
    size_t _capacity = 0;

    std::vector<Entity> _entity_of_index;
    std::unordered_map<uint32_t, size_t> _index_of_entity;
};

} // namespace gw
