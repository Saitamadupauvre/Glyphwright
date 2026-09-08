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

#define GW_EXPAND(x) x
#define GW_ARGCOUNT(...) GW_EXPAND(GW_ARGCOUNT_(__VA_ARGS__, 12,11,10,9,8,7,6,5,4,3,2,1))
#define GW_ARGCOUNT_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,N,...) N

#define GW_FIELDS_1(T,a) GW_FIELD(T,a)
#define GW_FIELDS_2(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_1(T,__VA_ARGS__))
#define GW_FIELDS_3(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_2(T,__VA_ARGS__))
#define GW_FIELDS_4(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_3(T,__VA_ARGS__))
#define GW_FIELDS_5(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_4(T,__VA_ARGS__))
#define GW_FIELDS_6(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_5(T,__VA_ARGS__))
#define GW_FIELDS_7(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_6(T,__VA_ARGS__))
#define GW_FIELDS_8(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_7(T,__VA_ARGS__))
#define GW_FIELDS_9(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_8(T,__VA_ARGS__))
#define GW_FIELDS_10(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_9(T,__VA_ARGS__))
#define GW_FIELDS_11(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_10(T,__VA_ARGS__))
#define GW_FIELDS_12(T,a,...) GW_FIELD(T,a), GW_EXPAND(GW_FIELDS_11(T,__VA_ARGS__))

#define GW_CAT(a, b) GW_CAT_(a, b)
#define GW_CAT_(a, b) a##b

#define GW_FIELDS_(N, T, ...) GW_EXPAND(GW_CAT(GW_FIELDS_, N)(T, __VA_ARGS__))
#define GW_FIELDS(T, ...) GW_FIELDS_(GW_ARGCOUNT(__VA_ARGS__), T, __VA_ARGS__)

#define ENGINE_REFLECT(Type, ...) \
    namespace { \
    const ::gw::ReflectRegistrar GW_CAT(gw_reflect_registrar_, __COUNTER__){ \
        ::gw::TypeInfo{#Type, sizeof(Type), alignof(Type), {GW_FIELDS(Type, __VA_ARGS__)}} \
    }; \
    }
