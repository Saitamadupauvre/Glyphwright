#pragma once
#include <cstdint>
#include <vector>
#include "gw/RendererVTable.hpp"

namespace gw {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool init(RendererRect rect) = 0;
    virtual void shutdown() = 0;
    virtual void setRect(RendererRect rect) = 0;
    virtual void beginFrame() = 0;
    virtual void drawBatch(const DrawCommand* commands, size_t count) = 0;
    virtual void endFrame() = 0;

    virtual bool supportsStateTransfer() const { return false; }
    virtual std::vector<uint8_t> serializeState() { return {}; }
    virtual bool deserializeState(const uint8_t* data, size_t size) { return false; }
};

} // namespace gw
