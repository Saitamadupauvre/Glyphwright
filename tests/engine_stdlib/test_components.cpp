#include <gtest/gtest.h>
#include "gw/Collider2D.hpp"
#include "gw/Reflection.hpp"
#include "gw/Sprite.hpp"
#include "gw/Velocity.hpp"

TEST(StdlibComponents, SpriteIsReflected) {
    const gw::TypeInfo* info = gw::ReflectionRegistry::instance().find("gw::Sprite");
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->size, sizeof(gw::Sprite));
    ASSERT_EQ(info->fields.size(), 4u);
    EXPECT_EQ(info->fields[0].name, "texture_id");
    EXPECT_EQ(info->fields[3].name, "layer");
}

TEST(StdlibComponents, VelocityIsReflected) {
    const gw::TypeInfo* info = gw::ReflectionRegistry::instance().find("gw::Velocity");
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->fields.size(), 2u);
    EXPECT_EQ(info->fields[0].name, "x");
    EXPECT_EQ(info->fields[1].name, "y");
}

TEST(StdlibComponents, Collider2DIsReflected) {
    const gw::TypeInfo* info = gw::ReflectionRegistry::instance().find("gw::Collider2D");
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->fields.size(), 3u);
    EXPECT_EQ(info->fields[0].name, "width");
    EXPECT_EQ(info->fields[1].name, "height");
    EXPECT_EQ(info->fields[2].name, "is_trigger");
}
