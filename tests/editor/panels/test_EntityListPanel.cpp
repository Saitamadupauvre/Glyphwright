#include <gtest/gtest.h>
#include "gw/editor/panels/EntityListPanel.hpp"

namespace {
struct Position {
    float x;
    float y;
};
} // namespace

TEST(EntityListPanel, EmptyWorldYieldsEmptyView) {
    gw::World world;
    gw::editor::EntityListPanel panel(world);
    EXPECT_TRUE(panel.view().rows.empty());
    EXPECT_EQ(panel.entityCount(), 0u);
}

TEST(EntityListPanel, ComponentlessEntityIsNotListed) {
    gw::World world;
    world.createEntity();
    gw::editor::EntityListPanel panel(world);
    EXPECT_TRUE(panel.view().rows.empty());
}

TEST(EntityListPanel, EntitiesWithComponentsAreListedInOrder) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});

    gw::editor::EntityListPanel panel(world);
    auto view = panel.view();
    ASSERT_EQ(view.rows.size(), 2u);
    EXPECT_EQ(view.rows[0].text, "Entity " + std::to_string(a.id));
    EXPECT_EQ(view.rows[1].text, "Entity " + std::to_string(b.id));
    EXPECT_TRUE(view.rows[0].children.empty());

    ASSERT_EQ(panel.entityCount(), 2u);
    EXPECT_EQ(panel.entityAt(0), a);
    EXPECT_EQ(panel.entityAt(1), b);
}
