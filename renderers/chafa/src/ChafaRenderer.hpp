#pragma once
#include <cstdint>
#include <vector>
#include "gw/IRenderer.hpp"

class ChafaRenderer final : public gw::IRenderer {
public:
    bool init(gw::RendererRect rect) override;
    void shutdown() override;
    void setRect(gw::RendererRect rect) override;
    void beginFrame() override;
    void drawBatch(const gw::DrawCommand* commands, size_t count) override;
    void endFrame() override;

    bool supportsStateTransfer() const override { return true; }
    std::vector<uint8_t> serializeState() override;
    bool deserializeState(const uint8_t* data, size_t size) override;

private:
    void resizeFramebuffer(gw::RendererRect rect);

    gw::RendererRect _rect{};
    std::vector<uint8_t> _framebuffer;
};
