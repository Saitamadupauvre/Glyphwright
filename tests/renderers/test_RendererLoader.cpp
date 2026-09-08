#include <gtest/gtest.h>
#include "gw/RendererLoader.hpp"

namespace {

constexpr gw::RendererRect kRect{0, 0, 4, 4};

TEST(RendererLoader, LoadNonexistentPathFails) {
    gw::RendererLoader loader;
    EXPECT_FALSE(loader.load("/nonexistent/path/renderer.so", kRect));
    EXPECT_EQ(loader.lastError(), gw::RendererLoadError::FileNotFound);
    EXPECT_FALSE(loader.isLoaded());
}

TEST(RendererLoader, LoadNoSymbolFixtureFails) {
    gw::RendererLoader loader;
    EXPECT_FALSE(loader.load(GW_TEST_RENDERER_NO_SYMBOL_PATH, kRect));
    EXPECT_EQ(loader.lastError(), gw::RendererLoadError::SymbolNotFound);
    EXPECT_FALSE(loader.isLoaded());
}

TEST(RendererLoader, LoadNullFieldFixtureFails) {
    gw::RendererLoader loader;
    EXPECT_FALSE(loader.load(GW_TEST_RENDERER_NULL_FIELD_PATH, kRect));
    EXPECT_EQ(loader.lastError(), gw::RendererLoadError::MissingRequiredFn);
    EXPECT_FALSE(loader.isLoaded());
}

TEST(RendererLoader, LoadGoodFixtureSucceeds) {
    gw::RendererLoader loader;
    EXPECT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    EXPECT_TRUE(loader.isLoaded());
    EXPECT_EQ(loader.lastError(), gw::RendererLoadError::None);
}

TEST(RendererLoader, DrawCallsAfterLoadDoNotCrash) {
    gw::RendererLoader loader;
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));

    loader.setRect(kRect);
    loader.beginFrame();
    loader.drawBatch(nullptr, 0);
    gw::DrawCommand one{gw::DrawCommandKind::FilledRect, 0, 0, 1, 1, 1, 1, 1, 1};
    loader.drawBatch(&one, 1);
    gw::DrawCommand many[64]{};
    loader.drawBatch(many, 64);
    loader.endFrame();
}

TEST(RendererLoader, RepeatedLoadReplacesCleanly) {
    gw::RendererLoader loader;
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    EXPECT_TRUE(loader.isLoaded());
    loader.drawBatch(nullptr, 0);
}

TEST(RendererLoader, SwapToGoodFixtureTransfersState) {
    gw::RendererLoader loader;
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    EXPECT_TRUE(loader.swap(GW_TEST_RENDERER_GOOD_PATH));
    EXPECT_TRUE(loader.isLoaded());
    loader.drawBatch(nullptr, 0);
}

TEST(RendererLoader, SwapToNonexistentPathLeavesOldRendererFunctional) {
    gw::RendererLoader loader;
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    EXPECT_FALSE(loader.swap("/nonexistent/path/renderer.so"));
    EXPECT_EQ(loader.lastError(), gw::RendererLoadError::FileNotFound);
    EXPECT_TRUE(loader.isLoaded());
    loader.beginFrame();
    loader.drawBatch(nullptr, 0);
    loader.endFrame();
}

TEST(RendererLoader, SwapToNullFieldFixtureLeavesOldRendererFunctional) {
    gw::RendererLoader loader;
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    EXPECT_FALSE(loader.swap(GW_TEST_RENDERER_NULL_FIELD_PATH));
    EXPECT_EQ(loader.lastError(), gw::RendererLoadError::MissingRequiredFn);
    EXPECT_TRUE(loader.isLoaded());
    loader.drawBatch(nullptr, 0);
}

TEST(RendererLoader, UnloadNeverLoadedIsNoop) {
    gw::RendererLoader loader;
    loader.unload();
    EXPECT_FALSE(loader.isLoaded());
}

TEST(RendererLoader, DoubleUnloadIsIdempotent) {
    gw::RendererLoader loader;
    ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    loader.unload();
    EXPECT_FALSE(loader.isLoaded());
    loader.unload();
    EXPECT_FALSE(loader.isLoaded());
}

TEST(RendererLoader, CallsOnUnloadedLoaderAreNoops) {
    gw::RendererLoader loader;
    loader.setRect(kRect);
    loader.beginFrame();
    loader.drawBatch(nullptr, 0);
    loader.endFrame();
    EXPECT_FALSE(loader.isLoaded());
}

TEST(RendererLoader, DestructorCleansUpLoadedRenderer) {
    {
        gw::RendererLoader loader;
        ASSERT_TRUE(loader.load(GW_TEST_RENDERER_GOOD_PATH, kRect));
    }
    SUCCEED();
}

} // namespace
