#include <gtest/gtest.h>
#include "gw/editor/model/EntityListModel.hpp"

namespace {
struct Position {
    float x;
    float y;
};
} // namespace

TEST(EntityListModel, EmptyWorldYieldsOnlyWorldRow) {
    gw::World world;
    auto list = gw::editor::buildEntityList(world);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_TRUE(list[0].isWorld);
    EXPECT_EQ(list[0].label, "World");
    EXPECT_EQ(list[0].entity, gw::kInvalidEntity);
}

TEST(EntityListModel, ComponentlessEntityIsNotListedButWorldRowIs) {
    gw::World world;
    world.createEntity();
    auto list = gw::editor::buildEntityList(world);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_TRUE(list[0].isWorld);
}

TEST(EntityListModel, EntitiesWithComponentsAreListedAfterWorldRow) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});

    auto list = gw::editor::buildEntityList(world);
    ASSERT_EQ(list.size(), 3u);
    EXPECT_TRUE(list[0].isWorld);
    EXPECT_EQ(list[1].entity, a);
    EXPECT_EQ(list[1].label, "Entity " + std::to_string(a.id));
    EXPECT_FALSE(list[1].isWorld);
    EXPECT_EQ(list[2].entity, b);
    EXPECT_EQ(list[2].label, "Entity " + std::to_string(b.id));
    EXPECT_FALSE(list[2].isWorld);
}
