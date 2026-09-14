#pragma once
#include <cstdint>
#include <cstddef>
#include "gw/DrawCommand.hpp"

namespace gw {

inline constexpr uint32_t kRendererAbiVersion = 1;

struct RendererRect {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
};

struct StateBuffer {
    const uint8_t* data;
    size_t size;
};

extern "C" {

using RendererCreateFn      = void* (*)();
using RendererDestroyFn     = void  (*)(void* self);
using RendererInitFn        = bool  (*)(void* self, RendererRect rect);
using RendererShutdownFn    = void  (*)(void* self);
using RendererSetRectFn     = void  (*)(void* self, RendererRect rect);
using RendererBeginFrameFn  = void  (*)(void* self);
using RendererDrawBatchFn   = void  (*)(void* self, const DrawCommand* commands, size_t count);
using RendererEndFrameFn    = void  (*)(void* self);
using RendererSerializeFn   = StateBuffer (*)(void* self);
using RendererFreeStateFn   = void  (*)(StateBuffer buf);
using RendererDeserializeFn = bool  (*)(void* self, const uint8_t* data, size_t size);

struct RendererVTable {
    uint32_t abiVersion;
    RendererCreateFn      create;
    RendererDestroyFn     destroy;
    RendererInitFn        init;
    RendererShutdownFn    shutdown;
    RendererSetRectFn     setRect;
    RendererBeginFrameFn  beginFrame;
    RendererDrawBatchFn   drawBatch;
    RendererEndFrameFn    endFrame;
    RendererSerializeFn   serializeState;
    RendererFreeStateFn   freeState;
    RendererDeserializeFn deserializeState;
};

using GetRendererVTableFn = const RendererVTable* (*)();

} // extern "C"

} // namespace gw
