#pragma once
#include <cstdint>
#include <vector>
#include "gw/Reflection.hpp"
#include "gw/World.hpp"
#include "gw/scene/SceneTypes.hpp"

namespace gw {

std::vector<uint8_t> serialize_scene_binary(const World& world, const ReflectionRegistry& registry,
                                             const std::vector<SceneComponentBinding>& bindings);

SceneDeserializeResult deserialize_scene_binary(World& world, const ReflectionRegistry& registry,
                                                 const std::vector<SceneComponentBinding>& bindings,
                                                 const std::vector<uint8_t>& bytes);

} // namespace gw
