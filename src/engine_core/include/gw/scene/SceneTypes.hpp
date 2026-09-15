#pragma once
#include <string>
#include <string_view>
#include <typeindex>
#include <vector>

namespace gw {

struct SceneComponentBinding {
    std::type_index type;
    std::string_view name;
};

struct SceneDeserializeResult {
    bool ok = true;
    std::vector<std::string> errors;
};

} // namespace gw

#define GW_SCENE_BINDING(Type) ::gw::SceneComponentBinding{std::type_index(typeid(Type)), #Type}
