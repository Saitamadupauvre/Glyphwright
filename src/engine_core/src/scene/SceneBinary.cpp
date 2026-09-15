#include "gw/scene/SceneBinary.hpp"
#include <cstring>
#include "SceneDetail.hpp"

namespace gw {

namespace {

constexpr uint32_t kMagic = 0x47575343; // "GWSC"
constexpr uint32_t kVersion = 1;

struct RawComponent {
    std::string name;
    std::vector<uint8_t> data;
};

struct RawEntity {
    uint32_t old_id = 0;
    uint32_t old_generation = 0;
    std::vector<RawComponent> components;
};

} // namespace

std::vector<uint8_t> serialize_scene_binary(const World& world, const ReflectionRegistry& registry,
                                             const std::vector<SceneComponentBinding>& bindings) {
    std::vector<uint8_t> out;

    auto writeRaw = [&](const void* p, size_t n) {
        const uint8_t* b = static_cast<const uint8_t*>(p);
        out.insert(out.end(), b, b + n);
    };
    auto writeU32 = [&](uint32_t v) { writeRaw(&v, sizeof(v)); };
    auto writeU16 = [&](uint16_t v) { writeRaw(&v, sizeof(v)); };

    writeU32(kMagic);
    writeU32(kVersion);

    std::vector<Entity> entities = world.allEntities();
    writeU32(static_cast<uint32_t>(entities.size()));

    for (Entity e : entities) {
        struct Resolved {
            const SceneComponentBinding* binding;
            const TypeInfo* info;
            const void* raw;
        };
        std::vector<Resolved> comps;
        for (const auto& b : bindings) {
            const void* raw = world.getComponentRaw(e, b.type);
            if (raw == nullptr) continue;
            const TypeInfo* info = registry.find(b.name);
            if (info == nullptr) continue;
            comps.push_back({&b, info, raw});
        }

        writeU32(e.id);
        writeU32(e.generation);
        writeU32(static_cast<uint32_t>(comps.size()));
        for (const auto& c : comps) {
            writeU16(static_cast<uint16_t>(c.binding->name.size()));
            writeRaw(c.binding->name.data(), c.binding->name.size());
            writeU32(static_cast<uint32_t>(c.info->size));
            writeRaw(c.raw, c.info->size);
        }
    }

    return out;
}

SceneDeserializeResult deserialize_scene_binary(World& world, const ReflectionRegistry& registry,
                                                 const std::vector<SceneComponentBinding>& bindings,
                                                 const std::vector<uint8_t>& bytes) {
    SceneDeserializeResult result;
    size_t pos = 0;

    auto fail = [&](std::string msg) {
        result.ok = false;
        result.errors.push_back(std::move(msg));
    };
    auto canRead = [&](size_t n) { return pos <= bytes.size() && n <= bytes.size() - pos; };
    auto readRaw = [&](void* dst, size_t n) -> bool {
        if (!canRead(n)) return false;
        std::memcpy(dst, bytes.data() + pos, n);
        pos += n;
        return true;
    };
    auto readU32 = [&](uint32_t& v) { return readRaw(&v, sizeof(v)); };
    auto readU16 = [&](uint16_t& v) { return readRaw(&v, sizeof(v)); };

    uint32_t magic = 0, version = 0, entityCount = 0;
    if (!readU32(magic) || magic != kMagic) {
        fail("malformed scene data: bad magic");
        return result;
    }
    if (!readU32(version) || version != kVersion) {
        fail("malformed scene data: unsupported version");
        return result;
    }
    if (!readU32(entityCount)) {
        fail("malformed scene data: truncated header");
        return result;
    }

    std::vector<RawEntity> rawEntities;
    rawEntities.reserve(entityCount);
    for (uint32_t i = 0; i < entityCount; ++i) {
        RawEntity re;
        uint32_t compCount = 0;
        if (!readU32(re.old_id) || !readU32(re.old_generation) || !readU32(compCount)) {
            fail("malformed scene data: truncated entity header");
            return result;
        }
        for (uint32_t c = 0; c < compCount; ++c) {
            uint16_t nameLen = 0;
            if (!readU16(nameLen) || !canRead(nameLen)) {
                fail("malformed scene data: truncated component name");
                return result;
            }
            std::string name(reinterpret_cast<const char*>(bytes.data() + pos), nameLen);
            pos += nameLen;

            uint32_t dataSize = 0;
            if (!readU32(dataSize) || !canRead(dataSize)) {
                fail("malformed scene data: truncated component data");
                return result;
            }
            std::vector<uint8_t> data(bytes.begin() + pos, bytes.begin() + pos + dataSize);
            pos += dataSize;

            re.components.push_back({std::move(name), std::move(data)});
        }
        rawEntities.push_back(std::move(re));
    }

    detail::EntityRemap remap;
    std::vector<Entity> newEntities;
    newEntities.reserve(rawEntities.size());
    for (const auto& re : rawEntities) {
        Entity ne = world.createEntity();
        remap[re.old_id] = ne;
        newEntities.push_back(ne);
    }

    for (size_t i = 0; i < rawEntities.size(); ++i) {
        Entity ne = newEntities[i];
        for (const auto& rc : rawEntities[i].components) {
            const TypeInfo* info = registry.find(rc.name);
            const SceneComponentBinding* binding = detail::findBinding(bindings, rc.name);
            if (info == nullptr || binding == nullptr) {
                fail("unknown component type: " + rc.name);
                continue;
            }
            if (rc.data.size() != info->size) {
                fail("component data size mismatch: " + rc.name);
                continue;
            }

            void* dst = world.addComponentRaw(ne, binding->type, info->size, info->align);
            std::memcpy(dst, rc.data.data(), rc.data.size());
            detail::applyEntityRemap(dst, *info, remap);
        }
    }

    return result;
}

} // namespace gw
