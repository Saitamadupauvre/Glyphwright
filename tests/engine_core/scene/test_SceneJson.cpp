#include <gtest/gtest.h>
#include "gw/scene/SceneJson.hpp"
#include "scene/SceneTestFixtures.hpp"

TEST(SceneJson, EmptyWorldRoundTrips) {
    gw::World world;
    auto& registry = gw::ReflectionRegistry::instance();
    auto json = gw::serialize_scene_json(world, registry, testBindings());

    gw::World loaded;
    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), json);

    EXPECT_TRUE(result.ok);
    EXPECT_TRUE(result.errors.empty());
    EXPECT_TRUE(loaded.allEntities().empty());
}

TEST(SceneJson, EntityWithZeroComponentsIsSkippedWithoutAffectingOthers) {
    gw::World world;
    world.createEntity();
    gw::Entity withComponent = world.createEntity();
    world.addComponent<Tag>(withComponent, Tag{5});

    auto& registry = gw::ReflectionRegistry::instance();
    auto json = gw::serialize_scene_json(world, registry, testBindings());
    EXPECT_NE(json.find("\"id\":"), std::string::npos);

    gw::World loaded;
    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), json);
    EXPECT_TRUE(result.ok);
    EXPECT_TRUE(result.errors.empty());

    auto entities = loaded.allEntities();
    ASSERT_EQ(entities.size(), 1u);
    Tag* tag = loaded.getComponent<Tag>(entities[0]);
    ASSERT_NE(tag, nullptr);
    EXPECT_EQ(tag->value, 5);
}

TEST(SceneJson, RoundTripReproducesFieldValues) {
    gw::World world;
    gw::Entity parent = world.createEntity();
    world.addComponent<Transform>(parent, Transform{1.0f, 2.0f});

    gw::Entity child = world.createEntity();
    world.addComponent<Transform>(child, Transform{3.5f, -4.5f});
    world.addComponent<Hierarchy>(child, Hierarchy{parent});
    world.addComponent<Tag>(child, Tag{42});

    auto& registry = gw::ReflectionRegistry::instance();
    auto json = gw::serialize_scene_json(world, registry, testBindings());

    gw::World loaded;
    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), json);
    ASSERT_TRUE(result.ok);

    auto entities = loaded.allEntities();
    ASSERT_EQ(entities.size(), 2u);

    gw::Entity newParent{}, newChild{};
    for (gw::Entity e : entities) {
        if (loaded.getComponent<Hierarchy>(e) != nullptr) newChild = e;
        else newParent = e;
    }

    Transform* parentTransform = loaded.getComponent<Transform>(newParent);
    ASSERT_NE(parentTransform, nullptr);
    EXPECT_FLOAT_EQ(parentTransform->x, 1.0f);
    EXPECT_FLOAT_EQ(parentTransform->y, 2.0f);

    Transform* childTransform = loaded.getComponent<Transform>(newChild);
    ASSERT_NE(childTransform, nullptr);
    EXPECT_FLOAT_EQ(childTransform->x, 3.5f);
    EXPECT_FLOAT_EQ(childTransform->y, -4.5f);

    Tag* tag = loaded.getComponent<Tag>(newChild);
    ASSERT_NE(tag, nullptr);
    EXPECT_EQ(tag->value, 42);
}

TEST(SceneJson, EntityReferenceSurvivesRemap) {
    gw::World world;
    gw::Entity parent = world.createEntity();
    world.addComponent<Tag>(parent, Tag{1});
    gw::Entity child = world.createEntity();
    world.addComponent<Hierarchy>(child, Hierarchy{parent});

    auto& registry = gw::ReflectionRegistry::instance();
    auto json = gw::serialize_scene_json(world, registry, testBindings());

    gw::World loaded;
    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), json);
    ASSERT_TRUE(result.ok);

    auto entities = loaded.allEntities();
    ASSERT_EQ(entities.size(), 2u);

    gw::Entity newParent{}, newChild{};
    for (gw::Entity e : entities) {
        if (loaded.getComponent<Hierarchy>(e) != nullptr) newChild = e;
        else newParent = e;
    }

    Hierarchy* hierarchy = loaded.getComponent<Hierarchy>(newChild);
    ASSERT_NE(hierarchy, nullptr);
    EXPECT_EQ(hierarchy->parent, newParent);
    EXPECT_NE(hierarchy->parent.id, parent.id);
}

TEST(SceneJson, DanglingParentReferenceBecomesInvalidEntity) {
    gw::World world;
    gw::Entity child = world.createEntity();
    world.addComponent<Hierarchy>(child, Hierarchy{gw::Entity{999, 0}});

    auto& registry = gw::ReflectionRegistry::instance();
    auto json = gw::serialize_scene_json(world, registry, testBindings());

    gw::World loaded;
    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), json);
    ASSERT_TRUE(result.ok);

    auto entities = loaded.allEntities();
    ASSERT_EQ(entities.size(), 1u);

    Hierarchy* hierarchy = loaded.getComponent<Hierarchy>(entities[0]);
    ASSERT_NE(hierarchy, nullptr);
    EXPECT_EQ(hierarchy->parent, gw::kInvalidEntity);
}

TEST(SceneJson, UnknownComponentTypeNameReportsErrorButKeepsEntity) {
    gw::World world;
    gw::Entity e = world.createEntity();
    world.addComponent<Transform>(e, Transform{1.0f, 1.0f});
    world.addComponent<Tag>(e, Tag{7});

    auto& registry = gw::ReflectionRegistry::instance();
    auto json = gw::serialize_scene_json(world, registry, testBindings());

    std::vector<gw::SceneComponentBinding> reducedBindings{GW_SCENE_BINDING(Transform), GW_SCENE_BINDING(Hierarchy)};

    gw::World loaded;
    auto result = gw::deserialize_scene_json(loaded, registry, reducedBindings, json);

    EXPECT_FALSE(result.ok);
    ASSERT_FALSE(result.errors.empty());

    auto entities = loaded.allEntities();
    ASSERT_EQ(entities.size(), 1u);
    Transform* transform = loaded.getComponent<Transform>(entities[0]);
    ASSERT_NE(transform, nullptr);
    EXPECT_FLOAT_EQ(transform->x, 1.0f);
}

TEST(SceneJson, MalformedJsonReportsErrorWithoutCrashing) {
    gw::World loaded;
    auto& registry = gw::ReflectionRegistry::instance();

    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), "{not valid json");

    EXPECT_FALSE(result.ok);
    ASSERT_FALSE(result.errors.empty());
    EXPECT_TRUE(loaded.allEntities().empty());
}

TEST(SceneJson, MissingEntitiesArrayReportsError) {
    gw::World loaded;
    auto& registry = gw::ReflectionRegistry::instance();

    auto result = gw::deserialize_scene_json(loaded, registry, testBindings(), "{}");

    EXPECT_FALSE(result.ok);
    ASSERT_FALSE(result.errors.empty());
}
