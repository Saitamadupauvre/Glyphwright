#include "gw/editor/model/InspectorModel.hpp"
#include <cstdint>
#include <cstring>

namespace gw::editor {

namespace {

std::string formatField(const std::byte* base, const FieldInfo& field) {
    const std::byte* addr = base + field.offset;
    switch (field.type) {
        case FieldType::I32: {
            int32_t v;
            std::memcpy(&v, addr, sizeof(v));
            return std::to_string(v);
        }
        case FieldType::U32: {
            uint32_t v;
            std::memcpy(&v, addr, sizeof(v));
            return std::to_string(v);
        }
        case FieldType::F32: {
            float v;
            std::memcpy(&v, addr, sizeof(v));
            return std::to_string(v);
        }
        case FieldType::F64: {
            double v;
            std::memcpy(&v, addr, sizeof(v));
            return std::to_string(v);
        }
        case FieldType::Bool: {
            bool v;
            std::memcpy(&v, addr, sizeof(v));
            return v ? "true" : "false";
        }
        case FieldType::Entity: {
            Entity v;
            std::memcpy(&v, addr, sizeof(v));
            return std::to_string(v.id);
        }
        case FieldType::String:
            return "<string>";
    }
    return "<unknown>";
}

} // namespace

std::vector<ComponentView> buildInspector(const World& world, Entity e,
                                           const ReflectionRegistry& registry,
                                           const std::vector<KnownType>& knownTypes) {
    std::vector<ComponentView> result;
    for (const auto& [type, name] : knownTypes) {
        const void* raw = world.getComponentRaw(e, type);
        if (raw == nullptr) continue;

        const TypeInfo* info = registry.find(name);
        if (info == nullptr) continue;

        ComponentView view;
        view.typeName = name;
        const std::byte* base = static_cast<const std::byte*>(raw);
        for (const FieldInfo& field : info->fields) {
            view.fields.push_back(FieldView{std::string(field.name), formatField(base, field)});
        }
        result.push_back(std::move(view));
    }
    return result;
}

} // namespace gw::editor
