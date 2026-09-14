#include "gw/Reflection.hpp"

namespace gw {

ReflectionRegistry& ReflectionRegistry::instance() {
    static ReflectionRegistry registry;
    return registry;
}

void ReflectionRegistry::registerType(TypeInfo info) {
    _types.push_back(std::move(info));
}

const TypeInfo* ReflectionRegistry::find(std::string_view name) const {
    for (const auto& t : _types) {
        if (t.name == name) return &t;
    }
    return nullptr;
}

} // namespace gw
