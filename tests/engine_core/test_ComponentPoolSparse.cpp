#include <gtest/gtest.h>
#include "gw/ComponentPool.hpp"

namespace {

struct Vec2 {
    float x;
    float y;
};

} // namespace

TEST(ComponentPoolSparse, GetWithStaleGenerationReturnsNull) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    pool.emplace(gw::Entity{3, 1});
    EXPECT_EQ(pool.get(gw::Entity{3, 0}), nullptr);
    EXPECT_NE(pool.get(gw::Entity{3, 1}), nullptr);
}

TEST(ComponentPoolSparse, RemoveWithStaleGenerationIsNoOp) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    pool.emplace(gw::Entity{3, 1});
    pool.remove(gw::Entity{3, 0});
    EXPECT_EQ(pool.size(), 1u);
}

TEST(ComponentPoolSparse, EmplaceTwiceForSameEntityKeepsOneSlotAndValue) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{2, 0};
    static_cast<Vec2*>(pool.emplace(e))->x = 5.0f;
    void* again = pool.emplace(e);

    EXPECT_EQ(pool.size(), 1u);
    EXPECT_EQ(again, pool.get(e));
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(again)->x, 5.0f);
}

TEST(ComponentPoolSparse, EmplaceWithNewGenerationReplacesOldOwner) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    pool.emplace(gw::Entity{2, 0});
    pool.emplace(gw::Entity{2, 1});

    EXPECT_EQ(pool.size(), 1u);
    EXPECT_EQ(pool.get(gw::Entity{2, 0}), nullptr);
    EXPECT_NE(pool.get(gw::Entity{2, 1}), nullptr);
}

TEST(ComponentPoolSparse, RemoveLastElementKeepsOthersIntact) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e1{1, 0}, e2{2, 0};
    static_cast<Vec2*>(pool.emplace(e1))->x = 1.0f;
    static_cast<Vec2*>(pool.emplace(e2))->x = 2.0f;
    pool.remove(e2);

    EXPECT_EQ(pool.size(), 1u);
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(pool.get(e1))->x, 1.0f);
}

TEST(ComponentPoolSparse, ReaddAfterRemoveWorks) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{1, 0};
    pool.emplace(e);
    pool.remove(e);
    EXPECT_EQ(pool.get(e), nullptr);
    pool.emplace(e);
    EXPECT_NE(pool.get(e), nullptr);
    EXPECT_EQ(pool.size(), 1u);
}

TEST(ComponentPoolSparse, SparseIdsFarApartWork) {
    gw::ComponentPool pool(sizeof(Vec2), alignof(Vec2));
    gw::Entity far{5000, 0};
    static_cast<Vec2*>(pool.emplace(far))->x = 8.0f;
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(pool.get(far))->x, 8.0f);
    EXPECT_EQ(pool.get(gw::Entity{4999, 0}), nullptr);
    EXPECT_EQ(pool.get(gw::Entity{900000, 0}), nullptr);
}

TEST(ComponentPoolSparse, MoveAssignTransfersOwnership) {
    gw::ComponentPool source(sizeof(Vec2), alignof(Vec2));
    gw::Entity e{1, 0};
    static_cast<Vec2*>(source.emplace(e))->x = 4.0f;

    gw::ComponentPool target(sizeof(Vec2), alignof(Vec2));
    target = std::move(source);

    ASSERT_NE(target.get(e), nullptr);
    EXPECT_FLOAT_EQ(static_cast<Vec2*>(target.get(e))->x, 4.0f);
    EXPECT_EQ(source.size(), 0u);
}
