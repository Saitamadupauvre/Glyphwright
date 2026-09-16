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

struct Velocity {
    float dx;
    float dy;
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

TEST(World, SingletonFirstAccessDefaultInits) {
    gw::World world;
    Position& p = world.singleton<Position>();
    EXPECT_FLOAT_EQ(p.x, 0.0f);
    EXPECT_FLOAT_EQ(p.y, 0.0f);
}

TEST(World, SingletonSecondAccessReturnsSameAddressAndValue) {
    gw::World world;
    Position& first = world.singleton<Position>();
    first.x = 9.0f;
    first.y = 10.0f;

    Position& second = world.singleton<Position>();
    EXPECT_EQ(&first, &second);
    EXPECT_FLOAT_EQ(second.x, 9.0f);
    EXPECT_FLOAT_EQ(second.y, 10.0f);
}

TEST(World, HasSingletonReflectsPresence) {
    gw::World world;
    EXPECT_FALSE(world.hasSingleton<Position>());
    world.singleton<Position>();
    EXPECT_TRUE(world.hasSingleton<Position>());
}

TEST(World, RemoveSingletonThenReaccessDefaultInits) {
    gw::World world;
    world.singleton<Position>().x = 5.0f;
    world.removeSingleton<Position>();
    EXPECT_FALSE(world.hasSingleton<Position>());

    Position& again = world.singleton<Position>();
    EXPECT_FLOAT_EQ(again.x, 0.0f);
}

TEST(World, RemoveSingletonOnUnknownTypeIsNoop) {
    gw::World world;
    world.removeSingleton<Position>();
    EXPECT_FALSE(world.hasSingleton<Position>());
}

TEST(World, TwoDistinctSingletonTypesAreIndependent) {
    gw::World world;
    world.singleton<Position>().x = 1.0f;
    world.singleton<Tag>().value = 42;

    EXPECT_FLOAT_EQ(world.singleton<Position>().x, 1.0f);
    EXPECT_EQ(world.singleton<Tag>().value, 42);
}

TEST(World, SingletonsExcludedFromAllEntities) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Tag>(e, Tag{1});
    world.singleton<Position>();

    auto entities = world.allEntities();
    EXPECT_EQ(entities.size(), 1u);
    EXPECT_EQ(entities[0], e);
}

TEST(World, SingletonsExcludedFromEach) {
    gw::World world;
    world.singleton<Position>();
    EXPECT_TRUE(world.each<Position>().empty());
}

TEST(World, SingletonTypesReturnsRegisteredTypes) {
    gw::World world;
    world.singleton<Position>();
    world.singleton<Tag>();

    auto types = world.singletonTypes();
    EXPECT_EQ(types.size(), 2u);
    EXPECT_NE(std::find(types.begin(), types.end(), std::type_index(typeid(Position))), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), std::type_index(typeid(Tag))), types.end());
}

TEST(World, SingletonRawRoundTripsAndMatchesTypedAccess) {
    gw::World world;
    void* raw = world.singletonRaw(std::type_index(typeid(Position)), sizeof(Position), alignof(Position));
    ASSERT_NE(raw, nullptr);
    static_cast<Position*>(raw)->x = 7.0f;

    EXPECT_FLOAT_EQ(world.singleton<Position>().x, 7.0f);

    const gw::World& constWorld = world;
    const void* constRaw = constWorld.getSingletonRaw(std::type_index(typeid(Position)));
    ASSERT_NE(constRaw, nullptr);
    EXPECT_FLOAT_EQ(static_cast<const Position*>(constRaw)->x, 7.0f);
}

TEST(World, GetSingletonRawReturnsNullForUnknownType) {
    gw::World world;
    EXPECT_EQ(world.getSingletonRaw(std::type_index(typeid(Position))), nullptr);
}

TEST(World, ViewOverEmptyWorldYieldsNothing) {
    gw::World world;
    int count = 0;
    for (auto tuple : world.view<Position, Velocity>()) {
        (void)tuple;
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(World, ViewWithMissingPoolYieldsEmpty) {
    gw::World world;
    gw::Entity a = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    // Velocity pool never created for any entity.

    int count = 0;
    for (auto tuple : world.view<Position, Velocity>()) {
        (void)tuple;
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(World, ViewFullOverlapYieldsAllEntities) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Velocity>(a, Velocity{0.1f, 0.1f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});
    world.addComponent<Velocity>(b, Velocity{0.2f, 0.2f});

    std::vector<gw::Entity> seen;
    for (auto [e, pos, vel] : world.view<Position, Velocity>()) {
        seen.push_back(e);
        pos.x += vel.dx;
    }

    EXPECT_EQ(seen.size(), 2u);
    EXPECT_NE(std::find(seen.begin(), seen.end(), a), seen.end());
    EXPECT_NE(std::find(seen.begin(), seen.end(), b), seen.end());
    EXPECT_FLOAT_EQ(world.getComponent<Position>(a)->x, 1.1f);
}

TEST(World, ViewPartialOverlapYieldsOnlyIntersection) {
    gw::World world;
    gw::Entity both = world.createEntity();
    gw::Entity onlyPosition = world.createEntity();
    gw::Entity onlyVelocity = world.createEntity();
    world.addComponent<Position>(both, Position{1.0f, 1.0f});
    world.addComponent<Velocity>(both, Velocity{0.1f, 0.1f});
    world.addComponent<Position>(onlyPosition, Position{2.0f, 2.0f});
    world.addComponent<Velocity>(onlyVelocity, Velocity{0.2f, 0.2f});

    std::vector<gw::Entity> seen;
    for (auto [e, pos, vel] : world.view<Position, Velocity>()) {
        (void)pos;
        (void)vel;
        seen.push_back(e);
    }

    EXPECT_EQ(seen.size(), 1u);
    EXPECT_EQ(seen[0], both);
}

TEST(World, EachCallbackFormMatchesView) {
    gw::World world;
    gw::Entity a = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Velocity>(a, Velocity{5.0f, 5.0f});

    int calls = 0;
    world.each<Position, Velocity>([&](gw::Entity e, Position& pos, Velocity& vel) {
        EXPECT_EQ(e, a);
        pos.x += vel.dx;
        ++calls;
    });

    EXPECT_EQ(calls, 1);
    EXPECT_FLOAT_EQ(world.getComponent<Position>(a)->x, 6.0f);
}

TEST(World, ViewRemovingUpcomingEntityMidIterationExcludesIt) {
    // Driver is the Position pool (tie-broken to the first template argument).
    // Removing a not-yet-visited entity's component in a non-driver pool
    // must exclude it from the remaining iteration.
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    gw::Entity c = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});
    world.addComponent<Position>(c, Position{3.0f, 3.0f});
    world.addComponent<Velocity>(a, Velocity{0.0f, 0.0f});
    world.addComponent<Velocity>(b, Velocity{0.0f, 0.0f});
    world.addComponent<Velocity>(c, Velocity{0.0f, 0.0f});

    std::vector<gw::Entity> seen;
    for (auto [e, pos, vel] : world.view<Position, Velocity>()) {
        (void)pos;
        (void)vel;
        seen.push_back(e);
        if (e == a) world.removeComponent<Velocity>(b);
    }

    EXPECT_EQ(seen.size(), 2u);
    EXPECT_NE(std::find(seen.begin(), seen.end(), a), seen.end());
    EXPECT_NE(std::find(seen.begin(), seen.end(), c), seen.end());
    EXPECT_EQ(std::find(seen.begin(), seen.end(), b), seen.end());
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
