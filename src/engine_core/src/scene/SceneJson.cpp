#include "gw/scene/SceneJson.hpp"
#include <cstring>
#include <sstream>
#include "JsonValue.hpp"
#include "SceneDetail.hpp"

namespace gw {

namespace {

void writeEscapedString(std::string& out, std::string_view s) {
    out += '"';
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            default: out += c; break;
        }
    }
    out += '"';
}

void writeFieldValue(std::string& out, const std::byte* base, const FieldInfo& field) {
    const std::byte* addr = base + field.offset;
    switch (field.type) {
        case FieldType::I32: {
            int32_t v;
            std::memcpy(&v, addr, sizeof(v));
            out += std::to_string(v);
            break;
        }
        case FieldType::U32: {
            uint32_t v;
            std::memcpy(&v, addr, sizeof(v));
            out += std::to_string(v);
            break;
        }
        case FieldType::F32: {
            float v;
            std::memcpy(&v, addr, sizeof(v));
            std::ostringstream ss;
            ss.precision(9);
            ss << v;
            out += ss.str();
            break;
        }
        case FieldType::F64: {
            double v;
            std::memcpy(&v, addr, sizeof(v));
            std::ostringstream ss;
            ss.precision(17);
            ss << v;
            out += ss.str();
            break;
        }
        case FieldType::Bool: {
            bool v;
            std::memcpy(&v, addr, sizeof(v));
            out += v ? "true" : "false";
            break;
        }
        case FieldType::Entity: {
            Entity v;
            std::memcpy(&v, addr, sizeof(v));
            out += std::to_string(v.id);
            break;
        }
    }
}

void readFieldValue(const detail::JsonObject& fields, std::byte* dst, const FieldInfo& field) {
    std::byte* addr = dst + field.offset;
    const detail::JsonValue* fv = detail::findMember(fields, field.name);

    switch (field.type) {
        case FieldType::I32: {
            int32_t v = 0;
            if (fv) {
                if (const double* n = detail::asNumber(*fv)) v = static_cast<int32_t>(*n);
            }
            std::memcpy(addr, &v, sizeof(v));
            break;
        }
        case FieldType::U32: {
            uint32_t v = 0;
            if (fv) {
                if (const double* n = detail::asNumber(*fv)) v = static_cast<uint32_t>(*n);
            }
            std::memcpy(addr, &v, sizeof(v));
            break;
        }
        case FieldType::F32: {
            float v = 0.0f;
            if (fv) {
                if (const double* n = detail::asNumber(*fv)) v = static_cast<float>(*n);
            }
            std::memcpy(addr, &v, sizeof(v));
            break;
        }
        case FieldType::F64: {
            double v = 0.0;
            if (fv) {
                if (const double* n = detail::asNumber(*fv)) v = *n;
            }
            std::memcpy(addr, &v, sizeof(v));
            break;
        }
        case FieldType::Bool: {
            bool v = false;
            if (fv) {
                if (const bool* b = detail::asBool(*fv)) v = *b;
            }
            std::memcpy(addr, &v, sizeof(v));
            break;
        }
        case FieldType::Entity: {
            Entity v{};
            if (fv) {
                if (const double* n = detail::asNumber(*fv)) v.id = static_cast<uint32_t>(*n);
            }
            std::memcpy(addr, &v, sizeof(v));
            break;
        }
    }
}

} // namespace

std::string serialize_scene_json(const World& world, const ReflectionRegistry& registry,
                                  const std::vector<SceneComponentBinding>& bindings) {
    std::string out;
    out += "{\"entities\":[";

    std::vector<Entity> entities = world.allEntities();
    for (size_t i = 0; i < entities.size(); ++i) {
        Entity e = entities[i];
        if (i > 0) out += ',';
        out += "{\"id\":" + std::to_string(e.id) + ",\"generation\":" + std::to_string(e.generation) +
               ",\"components\":[";

        bool firstComp = true;
        for (const auto& b : bindings) {
            const void* raw = world.getComponentRaw(e, b.type);
            if (raw == nullptr) continue;
            const TypeInfo* info = registry.find(b.name);
            if (info == nullptr) continue;

            if (!firstComp) out += ',';
            firstComp = false;

            out += "{\"type\":";
            writeEscapedString(out, b.name);
            out += ",\"fields\":{";
            const std::byte* base = static_cast<const std::byte*>(raw);
            for (size_t f = 0; f < info->fields.size(); ++f) {
                if (f > 0) out += ',';
                writeEscapedString(out, info->fields[f].name);
                out += ':';
                writeFieldValue(out, base, info->fields[f]);
            }
            out += "}}";
        }
        out += "]}";
    }

    out += "],\"singletons\":{";
    bool firstSingleton = true;
    for (const auto& b : bindings) {
        const void* raw = world.getSingletonRaw(b.type);
        if (raw == nullptr) continue;
        const TypeInfo* info = registry.find(b.name);
        if (info == nullptr) continue;

        if (!firstSingleton) out += ',';
        firstSingleton = false;

        writeEscapedString(out, b.name);
        out += ":{";
        const std::byte* base = static_cast<const std::byte*>(raw);
        for (size_t f = 0; f < info->fields.size(); ++f) {
            if (f > 0) out += ',';
            writeEscapedString(out, info->fields[f].name);
            out += ':';
            writeFieldValue(out, base, info->fields[f]);
        }
        out += "}";
    }
    out += "}}";
    return out;
}

SceneDeserializeResult deserialize_scene_json(World& world, const ReflectionRegistry& registry,
                                               const std::vector<SceneComponentBinding>& bindings,
                                               std::string_view json) {
    using namespace detail;

    SceneDeserializeResult result;
    auto fail = [&](std::string msg) {
        result.ok = false;
        result.errors.push_back(std::move(msg));
    };

    JsonValue root;
    if (!parseJson(json, root)) {
        fail("malformed scene JSON: parse error");
        return result;
    }

    const JsonObject* rootObj = asObject(root);
    if (rootObj == nullptr) {
        fail("malformed scene JSON: root is not an object");
        return result;
    }
    const JsonValue* entitiesVal = findMember(*rootObj, "entities");
    const JsonArray* entitiesArr = entitiesVal ? asArray(*entitiesVal) : nullptr;
    if (entitiesArr == nullptr) {
        fail("malformed scene JSON: missing or invalid \"entities\" array");
        return result;
    }

    struct RawComponent {
        std::string typeName;
        const JsonObject* fields;
    };
    struct RawEntity {
        uint32_t oldId = 0;
        std::vector<RawComponent> components;
    };

    std::vector<RawEntity> rawEntities;
    rawEntities.reserve(entitiesArr->size());
    for (const JsonValue& ev : *entitiesArr) {
        const JsonObject* eobj = asObject(ev);
        if (eobj == nullptr) {
            fail("malformed scene JSON: entity is not an object");
            return result;
        }
        const JsonValue* idVal = findMember(*eobj, "id");
        const double* idNum = idVal ? asNumber(*idVal) : nullptr;
        if (idNum == nullptr) {
            fail("malformed scene JSON: entity missing numeric \"id\"");
            return result;
        }

        RawEntity re;
        re.oldId = static_cast<uint32_t>(*idNum);

        const JsonValue* compsVal = findMember(*eobj, "components");
        const JsonArray* compsArr = compsVal ? asArray(*compsVal) : nullptr;
        if (compsArr == nullptr) {
            fail("malformed scene JSON: entity missing \"components\" array");
            return result;
        }
        for (const JsonValue& cv : *compsArr) {
            const JsonObject* cobj = asObject(cv);
            if (cobj == nullptr) {
                fail("malformed scene JSON: component is not an object");
                return result;
            }
            const JsonValue* typeVal = findMember(*cobj, "type");
            const std::string* typeName = typeVal ? asString(*typeVal) : nullptr;
            if (typeName == nullptr) {
                fail("malformed scene JSON: component missing \"type\"");
                return result;
            }
            const JsonValue* fieldsVal = findMember(*cobj, "fields");
            const JsonObject* fieldsObj = fieldsVal ? asObject(*fieldsVal) : nullptr;
            if (fieldsObj == nullptr) {
                fail("malformed scene JSON: component missing \"fields\" object");
                return result;
            }
            re.components.push_back({*typeName, fieldsObj});
        }
        rawEntities.push_back(std::move(re));
    }

    EntityRemap remap;
    std::vector<Entity> newEntities;
    newEntities.reserve(rawEntities.size());
    for (const auto& re : rawEntities) {
        Entity ne = world.createEntity();
        remap[re.oldId] = ne;
        newEntities.push_back(ne);
    }

    for (size_t i = 0; i < rawEntities.size(); ++i) {
        Entity ne = newEntities[i];
        for (const auto& rc : rawEntities[i].components) {
            const TypeInfo* info = registry.find(rc.typeName);
            const SceneComponentBinding* binding = findBinding(bindings, rc.typeName);
            if (info == nullptr || binding == nullptr) {
                fail("unknown component type: " + rc.typeName);
                continue;
            }

            void* dst = world.addComponentRaw(ne, binding->type, info->size, info->align);
            std::byte* base = static_cast<std::byte*>(dst);
            for (const FieldInfo& f : info->fields) {
                readFieldValue(*rc.fields, base, f);
            }
            applyEntityRemap(dst, *info, remap);
        }
    }

    const JsonValue* singletonsVal = findMember(*rootObj, "singletons");
    if (singletonsVal != nullptr) {
        const JsonObject* singletonsObj = asObject(*singletonsVal);
        if (singletonsObj == nullptr) {
            fail("malformed scene JSON: \"singletons\" is not an object");
        } else {
            for (const auto& [typeName, fieldsVal] : *singletonsObj) {
                const JsonObject* fieldsObj = asObject(fieldsVal);
                if (fieldsObj == nullptr) {
                    fail("malformed scene JSON: singleton \"" + typeName + "\" missing \"fields\" object");
                    continue;
                }
                const TypeInfo* info = registry.find(typeName);
                const SceneComponentBinding* binding = findBinding(bindings, typeName);
                if (info == nullptr || binding == nullptr) {
                    fail("unknown singleton type: " + typeName);
                    continue;
                }

                void* dst = world.singletonRaw(binding->type, info->size, info->align);
                std::byte* base = static_cast<std::byte*>(dst);
                for (const FieldInfo& f : info->fields) {
                    readFieldValue(*fieldsObj, base, f);
                }
                applyEntityRemap(dst, *info, remap);
            }
        }
    }

    return result;
}

} // namespace gw
