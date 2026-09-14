#pragma once
#include <cstdint>
#include <cstddef>

namespace gw {

enum class DrawCommandKind : uint32_t { FilledRect = 0 };

struct DrawCommand {
    DrawCommandKind kind;
    float x, y, w, h;
    uint8_t r, g, b, a;
};

struct DrawCommandBatch {
    const DrawCommand* commands;
    size_t count;
};

} // namespace gw
