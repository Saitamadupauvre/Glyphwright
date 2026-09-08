#include <gtest/gtest.h>
#include "gw/World.hpp"

namespace {

struct Position {
    float x;
    float y;
};

struct Tag {
    int value;
};

} // namespace

TEST(World, CreateEntityYieldsIncreasingIds) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    EXPECT_NE(a.id, b.id);
}

TEST(World, AddAndGetComponentRoundTrips) {
    gw::World world;
    gw::Entity e = world.createEntity();

    world.addComponent<Position>(e, Position{3.0f, 4.0f});

    Position* p = world.getComponent<Position>(e);
    ASSERT_NE(p, nullptr);
    EXPECT_FLOAT_EQ(p->x, 3.0f);
    EXPECT_FLOAT_EQ(p->y, 4.0f);
}

TEST(World, GetComponentOnEntityWithoutItReturnsNull) {
    gw::World world;
    gw::Entity e = world.createEntity();
    EXPECT_EQ(world.getComponent<Position>(e), nullptr);
}

TEST(World, RemoveComponentClearsIt) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{1.0f, 1.0f});
    world.removeComponent<Position>(e);
    EXPECT_EQ(world.getComponent<Position>(e), nullptr);
}

TEST(World, DestroyEntityRemovesFromAllPools) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{1.0f, 1.0f});
    world.addComponent<Tag>(e, Tag{42});

    world.destroyEntity(e);

    EXPECT_EQ(world.getComponent<Position>(e), nullptr);
    EXPECT_EQ(world.getComponent<Tag>(e), nullptr);
}

TEST(World, DistinctComponentTypesDoNotCollide) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{5.0f, 6.0f});
    world.addComponent<Tag>(e, Tag{7});

    EXPECT_EQ(world.getComponent<Tag>(e)->value, 7);
    EXPECT_FLOAT_EQ(world.getComponent<Position>(e)->x, 5.0f);
}
