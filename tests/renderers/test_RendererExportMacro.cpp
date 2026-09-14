#include <gtest/gtest.h>
#include <cstring>
#include "gw/RendererVTable.hpp"
#include "fixtures/TestRendererGood.hpp"

extern "C" const gw::RendererVTable* get_renderer_vtable();

namespace {

TEST(RendererExportMacro, AbiVersionMatches) {
    EXPECT_EQ(get_renderer_vtable()->abiVersion, gw::kRendererAbiVersion);
}

TEST(RendererExportMacro, AllRequiredFnPtrsNonNull) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    EXPECT_NE(vt->create, nullptr);
    EXPECT_NE(vt->destroy, nullptr);
    EXPECT_NE(vt->init, nullptr);
    EXPECT_NE(vt->shutdown, nullptr);
    EXPECT_NE(vt->setRect, nullptr);
    EXPECT_NE(vt->beginFrame, nullptr);
    EXPECT_NE(vt->drawBatch, nullptr);
    EXPECT_NE(vt->endFrame, nullptr);
}

TEST(RendererExportMacro, StateTransferTripletNonNull) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    EXPECT_NE(vt->serializeState, nullptr);
    EXPECT_NE(vt->freeState, nullptr);
    EXPECT_NE(vt->deserializeState, nullptr);
}

TEST(RendererExportMacro, CreateReturnsIndependentInstances) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    void* a = vt->create();
    void* b = vt->create();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_NE(a, b);
    vt->destroy(a);
    vt->destroy(b);
}

TEST(RendererExportMacro, DrawBatchHandlesZeroOneManyCommands) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    void* inst = vt->create();
    ASSERT_TRUE(vt->init(inst, gw::RendererRect{0, 0, 10, 10}));

    vt->drawBatch(inst, nullptr, 0);

    gw::DrawCommand one{gw::DrawCommandKind::FilledRect, 0, 0, 1, 1, 1, 1, 1, 1};
    vt->drawBatch(inst, &one, 1);

    gw::DrawCommand many[64]{};
    vt->drawBatch(inst, many, 64);

    vt->destroy(inst);
}

TEST(RendererExportMacro, SerializeDeserializeRoundTrip) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    void* a = vt->create();
    void* b = vt->create();
    ASSERT_TRUE(vt->init(a, gw::RendererRect{0, 0, 1, 1}));
    ASSERT_TRUE(vt->init(b, gw::RendererRect{0, 0, 1, 1}));

    gw::StateBuffer buf = vt->serializeState(a);
    ASSERT_NE(buf.data, nullptr);
    EXPECT_EQ(buf.size, sizeof(int32_t));

    ASSERT_TRUE(vt->deserializeState(b, buf.data, buf.size));
    vt->freeState(buf);

    auto* renderB = static_cast<TestRendererGood*>(b);
    EXPECT_EQ(renderB->swapGeneration, 1);

    vt->destroy(a);
    vt->destroy(b);
}

TEST(RendererExportMacro, FreeStateOnEmptyBufferIsSafe) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    vt->freeState(gw::StateBuffer{nullptr, 0});
}

TEST(RendererExportMacro, ShutdownThenDestroyDoesNotCrash) {
    const gw::RendererVTable* vt = get_renderer_vtable();
    void* inst = vt->create();
    ASSERT_TRUE(vt->init(inst, gw::RendererRect{0, 0, 1, 1}));
    vt->shutdown(inst);
    vt->destroy(inst);
}

} // namespace
