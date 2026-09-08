#include <gtest/gtest.h>
#include "gw/System.hpp"

namespace {

void system_a(gw::World&) {}
void system_b(gw::World&) {}
void system_c(gw::World&) {}

} // namespace

TEST(SystemRegistry, ResolveOrderFiltersByPhase) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Render, {}, {}});

    auto update = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(update.size(), 1u);
    EXPECT_EQ(update[0].name, "a");
}

TEST(SystemRegistry, RespectsAfterOrdering) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {"a"}, {}});
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {}});

    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0].name, "a");
    EXPECT_EQ(order[1].name, "b");
}

TEST(SystemRegistry, RespectsBeforeOrdering) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {"b"}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {}, {}});

    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0].name, "a");
    EXPECT_EQ(order[1].name, "b");
}

TEST(SystemRegistry, ChainOfThreeResolvesTransitively) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"c", &system_c, gw::Phase::Update, {"b"}, {}});
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {"a"}, {}});

    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0].name, "a");
    EXPECT_EQ(order[1].name, "b");
    EXPECT_EQ(order[2].name, "c");
}
