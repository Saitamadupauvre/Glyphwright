#include <gtest/gtest.h>
#include "gw/editor/panels/EntityListPanel.hpp"

namespace {
struct Position {
    float x;
    float y;
};
} // namespace

TEST(EntityListPanel, EmptyWorldYieldsOnlyWorldRow) {
    gw::World world;
    gw::editor::EntityListPanel panel(world);
    ASSERT_EQ(panel.view().rows.size(), 1u);
    EXPECT_EQ(panel.view().rows[0].text, "World");
    ASSERT_EQ(panel.entityCount(), 1u);
    EXPECT_EQ(panel.entityAt(0), gw::kInvalidEntity);
}

TEST(EntityListPanel, ComponentlessEntityIsNotListedButWorldRowIs) {
    gw::World world;
    world.createEntity();
    gw::editor::EntityListPanel panel(world);
    ASSERT_EQ(panel.view().rows.size(), 1u);
    EXPECT_EQ(panel.view().rows[0].text, "World");
}

TEST(EntityListPanel, EntitiesWithComponentsAreListedAfterWorldRow) {
    gw::World world;
    gw::Entity a = world.createEntity();
    gw::Entity b = world.createEntity();
    world.addComponent<Position>(a, Position{1.0f, 1.0f});
    world.addComponent<Position>(b, Position{2.0f, 2.0f});

    gw::editor::EntityListPanel panel(world);
    auto view = panel.view();
    ASSERT_EQ(view.rows.size(), 3u);
    EXPECT_EQ(view.rows[0].text, "World");
    EXPECT_EQ(view.rows[1].text, "Entity " + std::to_string(a.id));
    EXPECT_EQ(view.rows[2].text, "Entity " + std::to_string(b.id));
    EXPECT_TRUE(view.rows[1].children.empty());

    ASSERT_EQ(panel.entityCount(), 3u);
    EXPECT_EQ(panel.entityAt(0), gw::kInvalidEntity);
    EXPECT_EQ(panel.entityAt(1), a);
    EXPECT_EQ(panel.entityAt(2), b);
}
