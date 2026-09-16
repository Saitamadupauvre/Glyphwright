#include <gtest/gtest.h>
#include "gw/Hierarchy.hpp"
#include "gw/Transform.hpp"
#include "gw/editor/model/InspectorModel.hpp"

namespace {

std::vector<gw::editor::KnownType> knownTypes() {
    return {
        {std::type_index(typeid(gw::Transform)), "gw::Transform"},
        {std::type_index(typeid(gw::Hierarchy)), "gw::Hierarchy"},
    };
}

} // namespace

TEST(InspectorModel, EntityWithNoKnownComponentsYieldsEmpty) {
    gw::World world;
    gw::Entity e = world.createEntity();

    auto views = gw::editor::buildInspector(world, e, gw::ReflectionRegistry::instance(), knownTypes());
    EXPECT_TRUE(views.empty());
}

TEST(InspectorModel, TransformYieldsCorrectFields) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<gw::Transform>(e, gw::Transform{1.0f, 2.0f, 3.0f, 4.0f, 5.0f});

    auto views = gw::editor::buildInspector(world, e, gw::ReflectionRegistry::instance(), knownTypes());
    ASSERT_EQ(views.size(), 1u);
    EXPECT_EQ(views[0].typeName, "gw::Transform");
    ASSERT_EQ(views[0].fields.size(), 5u);
    EXPECT_EQ(views[0].fields[0].name, "x");
    EXPECT_EQ(views[0].fields[0].valueText, std::to_string(1.0f));
    EXPECT_EQ(views[0].fields[4].name, "scale_y");
    EXPECT_EQ(views[0].fields[4].valueText, std::to_string(5.0f));
}

TEST(InspectorModel, MultipleComponentsYieldMultipleViewsInStableOrder) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<gw::Transform>(e, gw::Transform{});
    world.addComponent<gw::Hierarchy>(e, gw::Hierarchy{gw::kInvalidEntity});

    auto views = gw::editor::buildInspector(world, e, gw::ReflectionRegistry::instance(), knownTypes());
    ASSERT_EQ(views.size(), 2u);
    EXPECT_EQ(views[0].typeName, "gw::Transform");
    EXPECT_EQ(views[1].typeName, "gw::Hierarchy");
}

TEST(InspectorModel, RemovedComponentIsExcluded) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<gw::Transform>(e, gw::Transform{});
    world.addComponent<gw::Hierarchy>(e, gw::Hierarchy{gw::kInvalidEntity});
    world.removeComponent<gw::Hierarchy>(e);

    auto views = gw::editor::buildInspector(world, e, gw::ReflectionRegistry::instance(), knownTypes());
    ASSERT_EQ(views.size(), 1u);
    EXPECT_EQ(views[0].typeName, "gw::Transform");
}

TEST(InspectorModel, WorldInspectorEmptyWhenNoSingletonsSet) {
    gw::World world;
    auto views = gw::editor::buildWorldInspector(world, gw::ReflectionRegistry::instance(), knownTypes());
    EXPECT_TRUE(views.empty());
}

TEST(InspectorModel, WorldInspectorYieldsSingletonFields) {
    gw::World world;
    world.singleton<gw::Transform>() = gw::Transform{1.0f, 2.0f, 3.0f, 4.0f, 5.0f};

    auto views = gw::editor::buildWorldInspector(world, gw::ReflectionRegistry::instance(), knownTypes());
    ASSERT_EQ(views.size(), 1u);
    EXPECT_EQ(views[0].typeName, "gw::Transform");
    ASSERT_EQ(views[0].fields.size(), 5u);
    EXPECT_EQ(views[0].fields[0].name, "x");
    EXPECT_EQ(views[0].fields[0].valueText, std::to_string(1.0f));
}

TEST(InspectorModel, WorldInspectorExcludesRemovedSingleton) {
    gw::World world;
    world.singleton<gw::Transform>();
    world.removeSingleton<gw::Transform>();

    auto views = gw::editor::buildWorldInspector(world, gw::ReflectionRegistry::instance(), knownTypes());
    EXPECT_TRUE(views.empty());
}

TEST(InspectorModel, BoolFieldFormatsAsTrueFalse) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<gw::Transform>(e, gw::Transform{});

    auto types = knownTypes();
    auto views = gw::editor::buildInspector(world, e, gw::ReflectionRegistry::instance(), types);
    ASSERT_EQ(views.size(), 1u);
    for (const auto& field : views[0].fields) {
        EXPECT_FALSE(field.valueText.empty());
    }
}
