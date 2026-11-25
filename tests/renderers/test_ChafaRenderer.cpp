#include <gtest/gtest.h>
#include <cstring>
#include "ChafaRenderer.hpp"

TEST(ChafaRenderer, ZeroSizedRectProducesNoFrameText) {
    ChafaRenderer renderer;
    ASSERT_TRUE(renderer.init(gw::RendererRect{0, 0, 0, 0}));
    renderer.beginFrame();
    renderer.drawBatch(nullptr, 0);
    renderer.endFrame();
    EXPECT_EQ(renderer.frameText(), nullptr);
}

TEST(ChafaRenderer, RectLargerThanFramebufferClipsDrawsAndProducesFrameText) {
    ChafaRenderer renderer;
    ASSERT_TRUE(renderer.init(gw::RendererRect{0, 0, 4, 4}));
    renderer.beginFrame();
    gw::DrawCommand oversized{gw::DrawCommandKind::FilledRect, -10, -10, 1000, 1000, 255, 0, 0, 255};
    renderer.drawBatch(&oversized, 1);
    renderer.endFrame();
    EXPECT_NE(renderer.frameText(), nullptr);
}

TEST(ChafaRenderer, SetRectBeforeInitResizesFramebuffer) {
    ChafaRenderer renderer;
    renderer.setRect(gw::RendererRect{0, 0, 8, 8});
    ASSERT_TRUE(renderer.init(gw::RendererRect{0, 0, 2, 2}));
    renderer.beginFrame();
    renderer.drawBatch(nullptr, 0);
    renderer.endFrame();
    EXPECT_NE(renderer.frameText(), nullptr);
}

TEST(ChafaRenderer, EndFrameBeforeAnyDrawProducesFrameText) {
    ChafaRenderer renderer;
    ASSERT_TRUE(renderer.init(gw::RendererRect{0, 0, 2, 2}));
    renderer.endFrame();
    EXPECT_NE(renderer.frameText(), nullptr);
}

TEST(ChafaRenderer, SetRectToZeroAfterNonZeroClearsFrameText) {
    ChafaRenderer renderer;
    ASSERT_TRUE(renderer.init(gw::RendererRect{0, 0, 4, 4}));
    renderer.beginFrame();
    renderer.endFrame();
    ASSERT_NE(renderer.frameText(), nullptr);

    renderer.setRect(gw::RendererRect{0, 0, 0, 0});
    renderer.beginFrame();
    renderer.endFrame();
    EXPECT_EQ(renderer.frameText(), nullptr);
}

TEST(ChafaRenderer, SerializeDeserializeRoundTripsRect) {
    ChafaRenderer a;
    ASSERT_TRUE(a.init(gw::RendererRect{1, 2, 5, 6}));
    auto state = a.serializeState();

    ChafaRenderer b;
    ASSERT_TRUE(b.init(gw::RendererRect{0, 0, 1, 1}));
    ASSERT_TRUE(b.deserializeState(state.data(), state.size()));

    b.beginFrame();
    b.endFrame();
    EXPECT_NE(b.frameText(), nullptr);
}
