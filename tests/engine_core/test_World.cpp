#include <gtest/gtest.h>
#include <algorithm>
#include <typeindex>
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

TEST(World, EachReturnsEmptyForUnknownType) {
    gw::World world;
    EXPECT_TRUE(world.each<Position>().empty());
}

TEST(World, EachReturnsAllEntitiesWithComponent) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    gw::Entity c = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});
    world.addComponent<Tag>(c, Tag{1});

    const auto& entities = world.each<Position>();
    EXPECT_EQ(entities.size(), 2u);
    EXPECT_NE(std::find(entities.begin(), entities.end(), a), entities.end());
    EXPECT_NE(std::find(entities.begin(), entities.end(), b), entities.end());
}

TEST(World, EachExcludesEntitiesAfterRemove) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{1.0f, 1.0f});
    world.removeComponent<Position>(e);

    EXPECT_TRUE(world.each<Position>().empty());
}

TEST(World, ComponentTypesOfReturnsEmptyForNoComponents) {
    gw::World world;
    gw::Entity e = world.createEntity();
    EXPECT_TRUE(world.componentTypesOf(e).empty());
}

TEST(World, ComponentTypesOfReturnsAllAddedTypes) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{1.0f, 1.0f});
    world.addComponent<Tag>(e, Tag{1});

    auto types = world.componentTypesOf(e);
    EXPECT_EQ(types.size(), 2u);
    EXPECT_NE(std::find(types.begin(), types.end(), std::type_index(typeid(Position))), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), std::type_index(typeid(Tag))), types.end());
}

TEST(World, ComponentTypesOfExcludesRemovedComponent) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{1.0f, 1.0f});
    world.addComponent<Tag>(e, Tag{1});
    world.removeComponent<Tag>(e);

    auto types = world.componentTypesOf(e);
    EXPECT_EQ(types.size(), 1u);
    EXPECT_EQ(types[0], std::type_index(typeid(Position)));
}

TEST(World, AllEntitiesReturnsUnionAcrossPools) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Tag>(b, Tag{1});

    auto entities = world.allEntities();
    EXPECT_EQ(entities.size(), 2u);
    EXPECT_NE(std::find(entities.begin(), entities.end(), a), entities.end());
    EXPECT_NE(std::find(entities.begin(), entities.end(), b), entities.end());
}

TEST(World, AllEntitiesDeduplicatesEntityInMultiplePools) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{1.0f, 1.0f});
    world.addComponent<Tag>(e, Tag{1});

    auto entities = world.allEntities();
    EXPECT_EQ(entities.size(), 1u);
    EXPECT_EQ(entities[0], e);
}

TEST(World, GetComponentRawReturnsNullForUnknownType) {
    gw::World world;
    gw::Entity e = world.createEntity();
    EXPECT_EQ(world.getComponentRaw(e, std::type_index(typeid(Position))), nullptr);
}

TEST(World, GetComponentRawRoundTripsWithAddComponent) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Position>(e, Position{3.0f, 4.0f});

    void* raw = world.getComponentRaw(e, std::type_index(typeid(Position)));
    ASSERT_NE(raw, nullptr);
    EXPECT_FLOAT_EQ(static_cast<Position*>(raw)->x, 3.0f);

    const gw::World& constWorld = world;
    const void* constRaw = constWorld.getComponentRaw(e, std::type_index(typeid(Position)));
    ASSERT_NE(constRaw, nullptr);
    EXPECT_FLOAT_EQ(static_cast<const Position*>(constRaw)->x, 3.0f);
}
