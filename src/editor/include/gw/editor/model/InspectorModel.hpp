#pragma once
#include <string>
#include <typeindex>
#include <utility>
#include <vector>
#include "gw/Entity.hpp"
#include "gw/Reflection.hpp"
#include "gw/World.hpp"

namespace gw::editor {

struct FieldView {
    std::string name;
    std::string valueText;
};

struct ComponentView {
    std::string typeName;
    std::vector<FieldView> fields;
};

using KnownType = std::pair<std::type_index, std::string>;

std::vector<ComponentView> buildInspector(const World& world, Entity e,
                                           const ReflectionRegistry& registry,
                                           const std::vector<KnownType>& knownTypes);

} // namespace gw::editor
