#include <gtest/gtest.h>
#include "gw/editor/model/EntityListModel.hpp"

namespace {
struct Position {
    float x;
    float y;
};
} // namespace

TEST(EntityListModel, EmptyWorldYieldsEmptyList) {
    gw::World world;
    EXPECT_TRUE(gw::editor::buildEntityList(world).empty());
}

TEST(EntityListModel, ComponentlessEntityIsNotListed) {
    gw::World world;
    world.createEntity();
    EXPECT_TRUE(gw::editor::buildEntityList(world).empty());
}

TEST(EntityListModel, EntitiesWithComponentsAreListed) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});

    auto list = gw::editor::buildEntityList(world);
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].entity, a);
    EXPECT_EQ(list[0].label, "Entity " + std::to_string(a.id));
    EXPECT_EQ(list[1].entity, b);
    EXPECT_EQ(list[1].label, "Entity " + std::to_string(b.id));
}
