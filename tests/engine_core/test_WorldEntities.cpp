#include <gtest/gtest.h>
#include <algorithm>
#include <stdexcept>
#include <typeindex>
#include <vector>
#include "gw/World.hpp"

namespace {

struct Tag {
    int value;
};

struct Position {
    float x;
    float y;
};

} // namespace

TEST(WorldEntities, NewEntityIsAliveAndCounted) {
    gw::World world;
    gw::Entity e = world.createEntity();
    EXPECT_TRUE(world.isAlive(e));
    EXPECT_EQ(world.aliveCount(), 1u);
}

TEST(WorldEntities, InvalidAndUnknownEntitiesAreNotAlive) {
    gw::World world;
    EXPECT_FALSE(world.isAlive(gw::kInvalidEntity));
    EXPECT_FALSE(world.isAlive(gw::Entity{99, 0}));
}

TEST(WorldEntities, DestroyedEntityIsNoLongerAlive) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.destroyEntity(e);
    EXPECT_FALSE(world.isAlive(e));
    EXPECT_EQ(world.aliveCount(), 0u);
}

TEST(WorldEntities, DestroyTwiceIsNoop) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.destroyEntity(a);
    world.destroyEntity(a);
    EXPECT_TRUE(world.isAlive(b));
    EXPECT_EQ(world.aliveCount(), 1u);
}

TEST(WorldEntities, DestroyInvalidEntityIsNoop) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.destroyEntity(gw::kInvalidEntity);
    world.destroyEntity(gw::Entity{500, 0});
    EXPECT_TRUE(world.isAlive(e));
    EXPECT_EQ(world.aliveCount(), 1u);
}

TEST(WorldEntities, DestroyedIdIsRecycledWithNewGeneration) {
    gw::World world;
    gw::Entity old = world.createEntity();
    world.destroyEntity(old);
    gw::Entity fresh = world.createEntity();

    EXPECT_EQ(fresh.id, old.id);
    EXPECT_EQ(fresh.generation, old.generation + 1);
    EXPECT_NE(fresh, old);
    EXPECT_TRUE(world.isAlive(fresh));
    EXPECT_FALSE(world.isAlive(old));
}

TEST(WorldEntities, StaleHandleCannotReadRecycledComponent) {
    gw::World world;
    gw::Entity old = world.createEntity();
    world.addComponent<Tag>(old, Tag{1});
    world.destroyEntity(old);
    gw::Entity fresh = world.createEntity();
    world.addComponent<Tag>(fresh, Tag{2});

    EXPECT_EQ(world.getComponent<Tag>(old), nullptr);
    ASSERT_NE(world.getComponent<Tag>(fresh), nullptr);
    EXPECT_EQ(world.getComponent<Tag>(fresh)->value, 2);
}

TEST(WorldEntities, StaleHandleDestroyDoesNotKillRecycledEntity) {
    gw::World world;
    gw::Entity old = world.createEntity();
    world.destroyEntity(old);
    gw::Entity fresh = world.createEntity();
    world.destroyEntity(old);
    EXPECT_TRUE(world.isAlive(fresh));
}

TEST(WorldEntities, StaleHandleRemoveComponentDoesNotAffectRecycledEntity) {
    gw::World world;
    gw::Entity old = world.createEntity();
    world.addComponent<Tag>(old, Tag{1});
    world.destroyEntity(old);
    gw::Entity fresh = world.createEntity();
    world.addComponent<Tag>(fresh, Tag{2});

    world.removeComponent<Tag>(old);
    EXPECT_NE(world.getComponent<Tag>(fresh), nullptr);
}

TEST(WorldEntities, AddComponentToDeadEntityThrows) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.destroyEntity(e);
    EXPECT_THROW(world.addComponent<Tag>(e, Tag{1}), std::invalid_argument);
    EXPECT_THROW(world.addComponent<Tag>(gw::kInvalidEntity, Tag{1}), std::invalid_argument);
}

TEST(WorldEntities, AddComponentRawToDeadEntityReturnsNull) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.destroyEntity(e);
    EXPECT_EQ(world.addComponentRaw(e, std::type_index(typeid(Tag)), sizeof(Tag), alignof(Tag)), nullptr);
}

TEST(WorldEntities, AddingSameComponentTwiceKeepsSingleEntry) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Tag>(e, Tag{1});
    world.addComponent<Tag>(e, Tag{2});

    EXPECT_EQ(world.each<Tag>().size(), 1u);
    EXPECT_EQ(world.getComponent<Tag>(e)->value, 2);
}

TEST(WorldEntities, AllEntitiesIncludesEntitiesWithoutComponents) {
    gw::World world;
    gw::Entity empty = world.createEntity();
    gw::Entity full = world.createEntity();
    world.addComponent<Tag>(full, Tag{1});

    auto entities = world.allEntities();
    ASSERT_EQ(entities.size(), 2u);
    EXPECT_EQ(entities[0], empty);
    EXPECT_EQ(entities[1], full);
}

TEST(WorldEntities, AllEntitiesExcludesDestroyedAndIsSortedById) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    gw::Entity c = world.createEntity();
    world.destroyEntity(b);

    auto entities = world.allEntities();
    ASSERT_EQ(entities.size(), 2u);
    EXPECT_EQ(entities[0], a);
    EXPECT_EQ(entities[1], c);
}
