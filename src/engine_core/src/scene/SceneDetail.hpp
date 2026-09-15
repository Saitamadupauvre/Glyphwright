#pragma once
#include <cstddef>
#include <cstring>
#include <unordered_map>
#include "gw/Entity.hpp"
#include "gw/Reflection.hpp"
#include "gw/scene/SceneTypes.hpp"

namespace gw::detail {

inline const SceneComponentBinding* findBinding(const std::vector<SceneComponentBinding>& bindings,
                                                  std::string_view name) {
    for (const auto& b : bindings) {
        if (b.name == name) return &b;
    }
    return nullptr;
}

using EntityRemap = std::unordered_map<uint32_t, Entity>;

inline void applyEntityRemap(void* component, const TypeInfo& info, const EntityRemap& remap) {
    for (const FieldInfo& f : info.fields) {
        if (f.type != FieldType::Entity) continue;
        std::byte* addr = static_cast<std::byte*>(component) + f.offset;
        Entity oldRef;
        std::memcpy(&oldRef, addr, sizeof(Entity));
        auto it = remap.find(oldRef.id);
        Entity fixed = (it != remap.end()) ? it->second : kInvalidEntity;
        std::memcpy(addr, &fixed, sizeof(Entity));
    }
}

} // namespace gw::detail
