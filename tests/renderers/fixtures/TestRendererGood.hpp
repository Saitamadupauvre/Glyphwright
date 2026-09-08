#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include "gw/IRenderer.hpp"

class TestRendererGood final : public gw::IRenderer {
public:
    bool init(gw::RendererRect rect) override {
        _rect = rect;
        ++initCount;
        return true;
    }
    void shutdown() override { ++shutdownCount; }
    void setRect(gw::RendererRect rect) override { _rect = rect; }
    void beginFrame() override { ++beginFrameCount; }
    void drawBatch(const gw::DrawCommand* commands, size_t count) override {
        ++drawBatchCallCount;
        lastBatchCount = count;
        (void)commands;
    }
    void endFrame() override { ++endFrameCount; }

    bool supportsStateTransfer() const override { return true; }

    std::vector<uint8_t> serializeState() override {
        std::vector<uint8_t> bytes(sizeof(int32_t));
        std::memcpy(bytes.data(), &swapGeneration, sizeof(int32_t));
        return bytes;
    }

    bool deserializeState(const uint8_t* data, size_t size) override {
        if (size != sizeof(int32_t)) return false;
        std::memcpy(&swapGeneration, data, sizeof(int32_t));
        ++swapGeneration;
        return true;
    }

    int initCount = 0;
    int shutdownCount = 0;
    int beginFrameCount = 0;
    int endFrameCount = 0;
    int drawBatchCallCount = 0;
    size_t lastBatchCount = 0;
    int32_t swapGeneration = 0;
    gw::RendererRect _rect{};
};
