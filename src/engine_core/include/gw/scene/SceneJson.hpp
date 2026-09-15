#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "gw/Reflection.hpp"
#include "gw/World.hpp"
#include "gw/scene/SceneTypes.hpp"

namespace gw {

std::string serialize_scene_json(const World& world, const ReflectionRegistry& registry,
                                  const std::vector<SceneComponentBinding>& bindings);

SceneDeserializeResult deserialize_scene_json(World& world, const ReflectionRegistry& registry,
                                               const std::vector<SceneComponentBinding>& bindings,
                                               std::string_view json);

} // namespace gw
