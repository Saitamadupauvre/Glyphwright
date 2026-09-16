#pragma once
#include <array>
#include <cstddef>
#include <tuple>
#include <utility>
#include "gw/ComponentPool.hpp"
#include "gw/Entity.hpp"

namespace gw {

template <typename... Ts>
class ComponentView {
    static constexpr size_t kCount = sizeof...(Ts);

public:
    ComponentView() = default;

    explicit ComponentView(std::array<ComponentPool*, kCount> pools) : _pools(pools) {
        _driver = _pools[0];
        for (size_t i = 1; i < kCount; ++i) {
            if (_pools[i]->size() < _driver->size()) _driver = _pools[i];
        }
    }

    class Iterator {
    public:
        Iterator(const ComponentView* view, size_t index) : _view(view), _index(index) {}

        Iterator& operator++() {
            ++_index;
            skipInvalid();
            return *this;
        }

        bool operator!=(const Iterator& other) const { return _index != other._index; }

        std::tuple<Entity, Ts&...> operator*() const {
            Entity e = _view->_driver->entities()[_index];
            return build(e, std::index_sequence_for<Ts...>{});
        }

    private:
        template <size_t... I>
        std::tuple<Entity, Ts&...> build(Entity e, std::index_sequence<I...>) const {
            return std::tuple<Entity, Ts&...>(e, *static_cast<Ts*>(_view->_pools[I]->get(e))...);
        }

        void skipInvalid() {
            const auto& entities = _view->_driver->entities();
            while (_index < entities.size() && !_view->allPresent(entities[_index])) ++_index;
        }

        const ComponentView* _view;
        size_t _index;

        friend class ComponentView;
    };

    Iterator begin() const {
        if (_driver == nullptr) return Iterator(this, 0);
        Iterator it(this, 0);
        it.skipInvalid();
        return it;
    }

    Iterator end() const { return Iterator(this, _driver == nullptr ? 0 : _driver->size()); }

private:
    template <size_t... I>
    bool allPresentImpl(Entity e, std::index_sequence<I...>) const {
        return (... && (_pools[I]->get(e) != nullptr));
    }

    bool allPresent(Entity e) const { return allPresentImpl(e, std::index_sequence_for<Ts...>{}); }

    std::array<ComponentPool*, kCount> _pools{};
    ComponentPool* _driver = nullptr;
};

} // namespace gw
