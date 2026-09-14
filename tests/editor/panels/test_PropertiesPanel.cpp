#include <gtest/gtest.h>
#include "gw/Hierarchy.hpp"
#include "gw/Transform.hpp"
#include "gw/editor/panels/PropertiesPanel.hpp"

namespace {

std::vector<gw::editor::KnownType> knownTypes() {
    return {
        {std::type_index(typeid(gw::Transform)), "gw::Transform"},
        {std::type_index(typeid(gw::Hierarchy)), "gw::Hierarchy"},
    };
}

} // namespace

TEST(PropertiesPanel, NoSelectionYieldsEmptyView) {
    gw::World world;
    gw::editor::PropertiesPanel panel(world, gw::ReflectionRegistry::instance(), knownTypes());
    EXPECT_TRUE(panel.view(std::nullopt).rows.empty());
}

TEST(PropertiesPanel, EntityWithNoKnownComponentsYieldsEmptyView) {
    gw::World world;
    gw::Entity e = world.createEntity();
    gw::editor::PropertiesPanel panel(world, gw::ReflectionRegistry::instance(), knownTypes());
    EXPECT_TRUE(panel.view(e).rows.empty());
}

TEST(PropertiesPanel, KnownComponentGroupsFieldsAsChildren) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<gw::Transform>(e, gw::Transform{1.0f, 2.0f, 3.0f, 4.0f, 5.0f});

    gw::editor::PropertiesPanel panel(world, gw::ReflectionRegistry::instance(), knownTypes());
    auto view = panel.view(e);
    ASSERT_EQ(view.rows.size(), 1u);
    EXPECT_EQ(view.rows[0].text, "gw::Transform");
    EXPECT_EQ(view.rows[0].style, gw::editor::RowStyle::Bold);
    ASSERT_EQ(view.rows[0].children.size(), 5u);
    EXPECT_EQ(view.rows[0].children[0].text, "x = " + std::to_string(1.0f));
}

TEST(PropertiesPanel, UnknownTypeNotOnEntityIsSkipped) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<gw::Transform>(e, gw::Transform{});

    gw::editor::PropertiesPanel panel(world, gw::ReflectionRegistry::instance(),
                                       {{std::type_index(typeid(gw::Hierarchy)), "gw::Hierarchy"}});
    EXPECT_TRUE(panel.view(e).rows.empty());
}
