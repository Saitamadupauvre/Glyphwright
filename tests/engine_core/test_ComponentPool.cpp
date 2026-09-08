#include <gtest/gtest.h>
#include "gw/ComponentPool.hpp"

namespace {

struct Vec2 {
    float x;
    float y;
};

} // namespace

TEST(ComponentPool, EmplaceAndGetRoundTrips) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{1, 0};

    auto* slot = static_cast<Vec2*>(pool.emplace(e));
    slot->x = 1.5f;
    slot->y = -2.5f;

    auto* fetched = static_cast<Vec2*>(pool.get(e));
    ASSERT_NE(fetched, nullptr);
    EXPECT_FLOAT_EQ(fetched->x, 1.5f);
    EXPECT_FLOAT_EQ(fetched->y, -2.5f);
    EXPECT_EQ(pool.size(), 1u);
}

TEST(ComponentPool, GetMissingEntityReturnsNull) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{42, 0};
    EXPECT_EQ(pool.get(e), nullptr);
}

TEST(ComponentPool, RemoveSwapsLastElementIntoHole) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e1{1, 0}, e2{2, 0}, e3{3, 0};

    static_cast<Vec2*>(pool.emplace(e1))->x = 1.0f;
    static_cast<Vec2*>(pool.emplace(e2))->x = 2.0f;
    static_cast<Vec2*>(pool.emplace(e3))->x = 3.0f;

    pool.remove(e1);

    EXPECT_EQ(pool.size(), 2u);
    EXPECT_EQ(pool.get(e1), nullptr);
    ASSERT_NE(pool.get(e2), nullptr);
    ASSERT_NE(pool.get(e3), nullptr);
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(pool.get(e2))->x, 2.0f);
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(pool.get(e3))->x, 3.0f);
}

TEST(ComponentPool, RemoveMissingEntityIsNoOp) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{1, 0};
    pool.remove(e);
    EXPECT_EQ(pool.size(), 0u);
}

TEST(ComponentPool, GrowsPastInitialCapacity) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    for (uint32_t i = 1; i <= 64; ++i) {
        gw::Entity e{i, 0};
        static_cast<Vec2*>(pool.emplace(e))->x = static_cast<float>(i);
    }
    EXPECT_EQ(pool.size(), 64u);
    gw::Entity last{64, 0};
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(pool.get(last))->x, 64.0f);
}

TEST(ComponentPool, MoveConstructTransfersOwnership) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{1, 0};
    static_cast<Vec2*>(pool.emplace(e))->x = 9.0f;

    gw::ComponentPool moved(std::move(pool));
    ASSERT_NE(moved.get(e), nullptr);
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(moved.get(e))->x, 9.0f);
}
