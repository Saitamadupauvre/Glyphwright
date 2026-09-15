#include <gtest/gtest.h>
#include "gw/System.hpp"
#include <stdexcept>
#include <string>

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

TEST(SystemRegistry, TwoNodeCycleThrows) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"b"}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {"a"}, {}});

    EXPECT_THROW(registry.resolveOrder(gw::Phase::Update), std::runtime_error);
}

TEST(SystemRegistry, CycleMessageListsSystems) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"b"}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {"a"}, {}});

    try {
        registry.resolveOrder(gw::Phase::Update);
        FAIL() << "expected throw";
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("a"), std::string::npos);
        EXPECT_NE(msg.find("b"), std::string::npos);
        EXPECT_NE(msg.find("cycle"), std::string::npos);
    }
}

TEST(SystemRegistry, ThreeNodeCycleThrows) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"c"}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {"a"}, {}});
    registry.registerSystem(gw::SystemInfo{"c", &system_c, gw::Phase::Update, {"b"}, {}});

    EXPECT_THROW(registry.resolveOrder(gw::Phase::Update), std::runtime_error);
}

TEST(SystemRegistry, BeforeAfterConflictThrows) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"b"}, {"b"}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {}, {}});

    EXPECT_THROW(registry.resolveOrder(gw::Phase::Update), std::runtime_error);
}

TEST(SystemRegistry, SelfReferenceAfterThrows) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"a"}, {}});

    EXPECT_THROW(registry.resolveOrder(gw::Phase::Update), std::runtime_error);
}

TEST(SystemRegistry, SelfReferenceBeforeThrows) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {"a"}});

    EXPECT_THROW(registry.resolveOrder(gw::Phase::Update), std::runtime_error);
}

TEST(SystemRegistry, CrossPhaseReferenceIgnored) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"r"}, {}});
    registry.registerSystem(gw::SystemInfo{"r", &system_b, gw::Phase::Render, {}, {"a"}});

    auto update = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(update.size(), 1u);
    EXPECT_EQ(update[0].name, "a");

    auto render = registry.resolveOrder(gw::Phase::Render);
    ASSERT_EQ(render.size(), 1u);
    EXPECT_EQ(render[0].name, "r");
}

TEST(SystemRegistry, UnknownReferenceIgnored) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {"nope"}, {"nope"}});

    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 1u);
    EXPECT_EQ(order[0].name, "a");
}

TEST(SystemRegistry, CycleInOtherPhaseDoesNotAffectThisPhase) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Render, {"b"}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Render, {"a"}, {}});
    registry.registerSystem(gw::SystemInfo{"c", &system_c, gw::Phase::Update, {}, {}});

    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 1u);
    EXPECT_EQ(order[0].name, "c");
}

TEST(SystemRegistry, IndependentSystemsKeepRegistrationOrder) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"c", &system_c, gw::Phase::Update, {}, {}});
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {}, {}});

    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0].name, "c");
    EXPECT_EQ(order[1].name, "a");
    EXPECT_EQ(order[2].name, "b");
}

TEST(SystemRegistry, TieBreakStableWithPartialConstraints) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"c", &system_c, gw::Phase::Update, {}, {}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {"a"}, {}});
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {}});

    auto first = registry.resolveOrder(gw::Phase::Update);
    auto second = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(first.size(), 3u);
    EXPECT_EQ(first[0].name, "c");
    EXPECT_EQ(first[1].name, "a");
    EXPECT_EQ(first[2].name, "b");
    for (size_t i = 0; i < first.size(); ++i) {
        EXPECT_EQ(first[i].name, second[i].name);
    }
}

TEST(SystemRegistry, ResolveIsIdempotentOnRegistry) {
    gw::SystemRegistry registry;
    registry.registerSystem(gw::SystemInfo{"a", &system_a, gw::Phase::Update, {}, {"b"}});
    registry.registerSystem(gw::SystemInfo{"b", &system_b, gw::Phase::Update, {}, {}});

    registry.resolveOrder(gw::Phase::Update);
    auto order = registry.resolveOrder(gw::Phase::Update);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[1].after.size(), 1u);
}

TEST(SystemRegistry, EmptyPhaseReturnsEmpty) {
    gw::SystemRegistry registry;
    EXPECT_TRUE(registry.resolveOrder(gw::Phase::Init).empty());
}

namespace {

void macro_none(gw::World&) {}
void macro_after(gw::World&) {}
void macro_before(gw::World&) {}
void macro_both(gw::World&) {}

} // namespace

ENGINE_SYSTEM(macro_none, gw::Phase::FixedUpdate)
ENGINE_SYSTEM(macro_after, gw::Phase::FixedUpdate, gw::After("macro_none"))
ENGINE_SYSTEM(macro_before, gw::Phase::FixedUpdate, gw::Before("macro_none"))
ENGINE_SYSTEM(macro_both, gw::Phase::FixedUpdate, gw::After("macro_before"), gw::Before("macro_none"))

TEST(SystemRegistry, MacroRegistersConstraintsInGlobalRegistry) {
    auto order = gw::SystemRegistry::instance().resolveOrder(gw::Phase::FixedUpdate);

    auto indexOf = [&](std::string_view name) -> size_t {
        for (size_t i = 0; i < order.size(); ++i) {
            if (order[i].name == name) return i;
        }
        return order.size();
    };

    ASSERT_LT(indexOf("macro_none"), order.size());
    ASSERT_LT(indexOf("macro_after"), order.size());
    ASSERT_LT(indexOf("macro_before"), order.size());
    ASSERT_LT(indexOf("macro_both"), order.size());

    EXPECT_LT(indexOf("macro_none"), indexOf("macro_after"));
    EXPECT_LT(indexOf("macro_before"), indexOf("macro_none"));
    EXPECT_LT(indexOf("macro_before"), indexOf("macro_both"));
    EXPECT_LT(indexOf("macro_both"), indexOf("macro_none"));
}

TEST(SystemRegistry, MakeSystemInfoFoldsConstraints) {
    auto info = gw::detail::make_system_info("x", &system_a, gw::Phase::Update,
                                             gw::After("p"), gw::After("q"), gw::Before("r"));
    ASSERT_EQ(info.after.size(), 2u);
    EXPECT_EQ(info.after[0], "p");
    EXPECT_EQ(info.after[1], "q");
    ASSERT_EQ(info.before.size(), 1u);
    EXPECT_EQ(info.before[0], "r");

    auto none = gw::detail::make_system_info("y", &system_a, gw::Phase::Update);
    EXPECT_TRUE(none.after.empty());
    EXPECT_TRUE(none.before.empty());
}
