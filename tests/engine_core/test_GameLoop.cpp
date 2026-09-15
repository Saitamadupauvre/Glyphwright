#include <gtest/gtest.h>
#include <algorithm>
#include "gw/GameLoop.hpp"
#include "gw/World.hpp"

namespace {

int g_initCalls = 0;
int g_fixedCalls = 0;
int g_updateCalls = 0;
int g_renderCalls = 0;
std::vector<std::string_view> g_order;

void reset_counters() {
    g_initCalls = 0;
    g_fixedCalls = 0;
    g_updateCalls = 0;
    g_renderCalls = 0;
    g_order.clear();
}

void loop_init(gw::World&) { ++g_initCalls; g_order.push_back("init"); }
void loop_fixed(gw::World&) { ++g_fixedCalls; g_order.push_back("fixed"); }
void loop_update(gw::World&) { ++g_updateCalls; g_order.push_back("update"); }
void loop_render(gw::World&) { ++g_renderCalls; g_order.push_back("render"); }

gw::SystemRegistry make_full_registry() {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"loop_init", &loop_init, gw::Phase::Init, {}, {}});
    registry.registerSystem(
        gw::SystemInfo{"loop_fixed", &loop_fixed, gw::Phase::FixedUpdate, {}, {}});
    registry.registerSystem(
        gw::SystemInfo{"loop_update", &loop_update, gw::Phase::Update, {}, {}});
    registry.registerSystem(
        gw::SystemInfo{"loop_render", &loop_render, gw::Phase::Render, {}, {}});
    return registry;
}

} // namespace

TEST(GameLoop, InitRunsOnceOnPlayNotPerFrame) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 1.0f / 60.0f, 0.25f);

    loop.play();
    EXPECT_EQ(g_initCalls, 1);

    loop.tick(1.0f / 60.0f);
    loop.tick(1.0f / 60.0f);
    EXPECT_EQ(g_initCalls, 1);
}

TEST(GameLoop, FixedUpdateRunsMatchingAccumulatedTime) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 0.1f, 10.0f);
    loop.play();
    reset_counters();

    loop.tick(0.35f);
    EXPECT_EQ(g_fixedCalls, 3);
    EXPECT_EQ(g_updateCalls, 1);
    EXPECT_EQ(g_renderCalls, 1);
}

TEST(GameLoop, ZeroDtRunsUpdateRenderOnceNoFixedUpdate) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 0.1f, 10.0f);
    loop.play();
    reset_counters();

    loop.tick(0.0f);
    EXPECT_EQ(g_fixedCalls, 0);
    EXPECT_EQ(g_updateCalls, 1);
    EXPECT_EQ(g_renderCalls, 1);
}

TEST(GameLoop, HugeDtClampedBySpiralOfDeathCap) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 0.1f, 1.0f);
    loop.play();
    reset_counters();

    loop.tick(1000.0f);
    EXPECT_LE(g_fixedCalls, 10);
    EXPECT_GE(g_fixedCalls, 9);
    EXPECT_EQ(g_updateCalls, 1);
    EXPECT_EQ(g_renderCalls, 1);
}

TEST(GameLoop, PlayTwiceWithoutStopDoesNotRerunInit) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 1.0f / 60.0f, 0.25f);

    loop.play();
    loop.play();
    EXPECT_EQ(g_initCalls, 1);
    EXPECT_EQ(loop.state(), gw::GameLoop::State::Playing);
}

TEST(GameLoop, StopWhilePausedTransitionsToStoppedAndBlocksTick) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 0.1f, 10.0f);

    loop.play();
    loop.pause();
    ASSERT_EQ(loop.state(), gw::GameLoop::State::Paused);

    loop.stop();
    EXPECT_EQ(loop.state(), gw::GameLoop::State::Stopped);

    reset_counters();
    loop.tick(1.0f);
    EXPECT_EQ(g_fixedCalls, 0);
    EXPECT_EQ(g_updateCalls, 0);
    EXPECT_EQ(g_renderCalls, 0);
}

TEST(GameLoop, EmptyRegistryPhaseTicksWithoutCrashing) {
    gw::World world;
    gw::SystemRegistry registry;
    gw::GameLoop loop(world, registry, 0.1f, 10.0f);

    loop.play();
    EXPECT_NO_THROW(loop.tick(0.05f));
    loop.stop();
}

TEST(GameLoop, SystemsExecuteInResolveOrderPerPhase) {
    reset_counters();
    gw::World world;
    gw::SystemRegistry registry = make_full_registry();
    gw::GameLoop loop(world, registry, 0.1f, 10.0f);

    loop.play();
    loop.tick(0.1f);

    ASSERT_FALSE(g_order.empty());
    EXPECT_EQ(g_order.front(), "init");
    auto fixedIt = std::find(g_order.begin(), g_order.end(), "fixed");
    auto updateIt = std::find(g_order.begin(), g_order.end(), "update");
    auto renderIt = std::find(g_order.begin(), g_order.end(), "render");
    ASSERT_NE(fixedIt, g_order.end());
    ASSERT_NE(updateIt, g_order.end());
    ASSERT_NE(renderIt, g_order.end());
    EXPECT_LT(fixedIt, updateIt);
    EXPECT_LT(updateIt, renderIt);
}
