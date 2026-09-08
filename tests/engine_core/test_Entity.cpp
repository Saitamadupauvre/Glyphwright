#include <gtest/gtest.h>
#include "gw/Entity.hpp"

TEST(Entity, EqualityComparesIdAndGeneration) {
    gw::Entity a{1, 0};
    gw::Entity b{1, 0};
    gw::Entity c{1, 1};
    gw::Entity d{2, 0};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST(Entity, InvalidEntityIsZero) {
    EXPECT_EQ(gw::kInvalidEntity.id, 0u);
    EXPECT_EQ(gw::kInvalidEntity.generation, 0u);
}
