#pragma once
#include <cstring>
#include "gw/IRenderer.hpp"
#include "gw/RendererVTable.hpp"

#define GW_RENDERER_CAT(a, b) GW_RENDERER_CAT_(a, b)
#define GW_RENDERER_CAT_(a, b) a##b

#define ENGINE_EXPORT_RENDERER(Type) \
    namespace { \
    void* gw_renderer_create_##Type() { return static_cast<void*>(new Type()); } \
    void gw_renderer_destroy_##Type(void* self) { delete static_cast<Type*>(self); } \
    bool gw_renderer_init_##Type(void* self, ::gw::RendererRect rect) { \
        return static_cast<Type*>(self)->init(rect); \
    } \
    void gw_renderer_shutdown_##Type(void* self) { static_cast<Type*>(self)->shutdown(); } \
    void gw_renderer_setrect_##Type(void* self, ::gw::RendererRect rect) { \
        static_cast<Type*>(self)->setRect(rect); \
    } \
    void gw_renderer_beginframe_##Type(void* self) { static_cast<Type*>(self)->beginFrame(); } \
    void gw_renderer_drawbatch_##Type(void* self, const ::gw::DrawCommand* cmds, size_t count) { \
        static_cast<Type*>(self)->drawBatch(cmds, count); \
    } \
    void gw_renderer_endframe_##Type(void* self) { static_cast<Type*>(self)->endFrame(); } \
    ::gw::StateBuffer gw_renderer_serialize_##Type(void* self) { \
        auto bytes = static_cast<Type*>(self)->serializeState(); \
        if (bytes.empty()) return ::gw::StateBuffer{nullptr, 0}; \
        auto* buf = new uint8_t[bytes.size()]; \
        std::memcpy(buf, bytes.data(), bytes.size()); \
        return ::gw::StateBuffer{buf, bytes.size()}; \
    } \
    void gw_renderer_freestate_##Type(::gw::StateBuffer buf) { delete[] buf.data; } \
    bool gw_renderer_deserialize_##Type(void* self, const uint8_t* data, size_t size) { \
        return static_cast<Type*>(self)->deserializeState(data, size); \
    } \
    } \
    extern "C" const ::gw::RendererVTable* get_renderer_vtable() { \
        static const Type gw_renderer_probe_##Type{}; \
        static const bool gw_renderer_supports_state_##Type = gw_renderer_probe_##Type.supportsStateTransfer(); \
        static const ::gw::RendererVTable table{ \
            ::gw::kRendererAbiVersion, \
            &gw_renderer_create_##Type, \
            &gw_renderer_destroy_##Type, \
            &gw_renderer_init_##Type, \
            &gw_renderer_shutdown_##Type, \
            &gw_renderer_setrect_##Type, \
            &gw_renderer_beginframe_##Type, \
            &gw_renderer_drawbatch_##Type, \
            &gw_renderer_endframe_##Type, \
            gw_renderer_supports_state_##Type ? &gw_renderer_serialize_##Type : nullptr, \
            gw_renderer_supports_state_##Type ? &gw_renderer_freestate_##Type : nullptr, \
            gw_renderer_supports_state_##Type ? &gw_renderer_deserialize_##Type : nullptr, \
        }; \
        return &table; \
    }
