#pragma once
#include <vector>
#include "gw/reflect_macro.hpp"
#include "gw/scene/SceneTypes.hpp"

namespace {

struct Transform {
    float x = 0.0f;
    float y = 0.0f;
};

struct Hierarchy {
    gw::Entity parent = gw::kInvalidEntity;
};

struct Tag {
    int32_t value = 0;
};

const gw::ReflectRegistrar transformRegistrar{
    gw::TypeInfo{"Transform", sizeof(Transform), alignof(Transform), {GW_FIELD(Transform, x), GW_FIELD(Transform, y)}}};
const gw::ReflectRegistrar hierarchyRegistrar{
    gw::TypeInfo{"Hierarchy", sizeof(Hierarchy), alignof(Hierarchy), {GW_FIELD(Hierarchy, parent)}}};
const gw::ReflectRegistrar tagRegistrar{
    gw::TypeInfo{"Tag", sizeof(Tag), alignof(Tag), {GW_FIELD(Tag, value)}}};

std::vector<gw::SceneComponentBinding> testBindings() {
    return {GW_SCENE_BINDING(Transform), GW_SCENE_BINDING(Hierarchy), GW_SCENE_BINDING(Tag)};
}

} // namespace
