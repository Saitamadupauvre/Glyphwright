#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#include <malloc.h>
#endif
#include <vector>
#include "gw/Entity.hpp"

namespace gw {

class ComponentPool {
public:
    ComponentPool(size_t element_size, size_t element_align)
        : _element_size(element_size), _element_align(element_align) {}

    ~ComponentPool() {
        if (_data) {
            alignedFree(_data);
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
          _sparse(std::move(other._sparse)) {
        other._data = nullptr;
        other._count = 0;
        other._capacity = 0;
    }

    ComponentPool& operator=(ComponentPool&& other) noexcept {
        if (this == &other) return *this;
        if (_data) alignedFree(_data);
        _element_size = other._element_size;
        _element_align = other._element_align;
        _data = other._data;
        _count = other._count;
        _capacity = other._capacity;
        _entity_of_index = std::move(other._entity_of_index);
        _sparse = std::move(other._sparse);
        other._data = nullptr;
        other._count = 0;
        other._capacity = 0;
        return *this;
    }

    void* emplace(Entity e) {
        size_t existing = indexOf(e.id);
        if (existing != kAbsent) {
            _entity_of_index[existing] = e;
            return elementAt(existing);
        }
        if (_count == _capacity) grow();
        size_t index = _count++;
        _entity_of_index.push_back(e);
        if (e.id >= _sparse.size()) _sparse.resize(static_cast<size_t>(e.id) + 1, kAbsentIndex);
        _sparse[e.id] = static_cast<uint32_t>(index);
        return elementAt(index);
    }

    void remove(Entity e) {
        size_t index = indexOfExact(e);
        if (index == kAbsent) return;
        size_t last = _count - 1;
        if (index != last) {
            std::memcpy(elementAt(index), elementAt(last), _element_size);
            Entity moved = _entity_of_index[last];
            _entity_of_index[index] = moved;
            _sparse[moved.id] = static_cast<uint32_t>(index);
        }
        _entity_of_index.pop_back();
        _sparse[e.id] = kAbsentIndex;
        --_count;
    }

    void* get(Entity e) {
        size_t index = indexOfExact(e);
        if (index == kAbsent) return nullptr;
        return elementAt(index);
    }

    const void* get(Entity e) const {
        size_t index = indexOfExact(e);
        if (index == kAbsent) return nullptr;
        return elementAt(index);
    }

    void* rawData() { return _data; }
    size_t size() const { return _count; }
    const std::vector<Entity>& entities() const { return _entity_of_index; }

private:
    static constexpr size_t kAbsent = static_cast<size_t>(-1);
    static constexpr uint32_t kAbsentIndex = 0xFFFFFFFFu;

    size_t indexOf(uint32_t id) const {
        if (id >= _sparse.size()) return kAbsent;
        uint32_t index = _sparse[id];
        return index == kAbsentIndex ? kAbsent : index;
    }

    size_t indexOfExact(Entity e) const {
        size_t index = indexOf(e.id);
        if (index == kAbsent || _entity_of_index[index] != e) return kAbsent;
        return index;
    }

    void grow() {
        size_t new_capacity = _capacity == 0 ? 16 : _capacity * 2;
        void* new_data = alignedAlloc(_element_align, alignUp(new_capacity * _element_size, _element_align));
        if (_data) {
            std::memcpy(new_data, _data, _count * _element_size);
            alignedFree(_data);
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

    static void* alignedAlloc(size_t align, size_t size) {
#ifdef _WIN32
        return _aligned_malloc(size, align);
#else
        return std::aligned_alloc(align, size);
#endif
    }

    static void alignedFree(void* ptr) {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
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
    std::vector<uint32_t> _sparse;
};

} // namespace gw
