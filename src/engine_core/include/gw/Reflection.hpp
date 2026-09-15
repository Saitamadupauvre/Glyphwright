#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

namespace gw {

enum class FieldType { I32, U32, F32, F64, Bool, Entity };

struct FieldInfo {
    std::string_view name;
    FieldType type;
    size_t offset;
    size_t size;
};

struct TypeInfo {
    std::string_view name;
    size_t size;
    size_t align;
    std::vector<FieldInfo> fields;
};

class ReflectionRegistry {
public:
    static ReflectionRegistry& instance();

    void registerType(TypeInfo info);
    const TypeInfo* find(std::string_view name) const;
    const std::vector<TypeInfo>& all() const { return _types; }

private:
    std::vector<TypeInfo> _types;
};

struct ReflectRegistrar {
    explicit ReflectRegistrar(TypeInfo info) {
        ReflectionRegistry::instance().registerType(std::move(info));
    }
};

} // namespace gw
