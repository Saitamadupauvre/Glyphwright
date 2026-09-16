#pragma once
#include <cstddef>
#include <cstdint>
#include "gw/Entity.hpp"
#include "gw/Reflection.hpp"

namespace gw::detail {

template <typename T> struct FieldTypeOf;
template <> struct FieldTypeOf<int32_t> { static constexpr FieldType value = FieldType::I32; };
template <> struct FieldTypeOf<uint32_t> { static constexpr FieldType value = FieldType::U32; };
template <> struct FieldTypeOf<float> { static constexpr FieldType value = FieldType::F32; };
template <> struct FieldTypeOf<double> { static constexpr FieldType value = FieldType::F64; };
template <> struct FieldTypeOf<bool> { static constexpr FieldType value = FieldType::Bool; };
template <> struct FieldTypeOf<::gw::Entity> { static constexpr FieldType value = FieldType::Entity; };

} // namespace gw::detail

#define GW_FIELD(Type, field) \
    ::gw::FieldInfo{ \
        #field, \
        ::gw::detail::FieldTypeOf<decltype(Type::field)>::value, \
        offsetof(Type, field), \
        sizeof(decltype(Type::field)) \
    }

// Recursive __VA_OPT__ expansion (C++20), depth 4^4=256 fields — no hardcoded field-count ceiling.
#define GW_PARENS ()

#define GW_EXPAND(...) GW_EXPAND4(GW_EXPAND4(GW_EXPAND4(GW_EXPAND4(__VA_ARGS__))))
#define GW_EXPAND4(...) GW_EXPAND3(GW_EXPAND3(GW_EXPAND3(GW_EXPAND3(__VA_ARGS__))))
#define GW_EXPAND3(...) GW_EXPAND2(GW_EXPAND2(GW_EXPAND2(GW_EXPAND2(__VA_ARGS__))))
#define GW_EXPAND2(...) GW_EXPAND1(GW_EXPAND1(GW_EXPAND1(GW_EXPAND1(__VA_ARGS__))))
#define GW_EXPAND1(...) __VA_ARGS__

#define GW_CAT(a, b) GW_CAT_(a, b)
#define GW_CAT_(a, b) a##b

#define GW_FIELDS_HELPER(T, a, ...) \
    GW_FIELD(T, a) __VA_OPT__(, GW_FIELDS_AGAIN GW_PARENS (T, __VA_ARGS__))
#define GW_FIELDS_AGAIN() GW_FIELDS_HELPER

#define GW_FIELDS(T, ...) __VA_OPT__(GW_EXPAND(GW_FIELDS_HELPER(T, __VA_ARGS__)))

#define ENGINE_REFLECT(Type, ...) \
    namespace { \
    const ::gw::ReflectRegistrar GW_CAT(gw_reflect_registrar_, __COUNTER__){ \
        ::gw::TypeInfo{#Type, sizeof(Type), alignof(Type), {GW_FIELDS(Type, __VA_ARGS__)}} \
    }; \
    }
