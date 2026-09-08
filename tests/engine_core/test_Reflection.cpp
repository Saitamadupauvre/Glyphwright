#include <gtest/gtest.h>
#include "gw/Reflection.hpp"
#include "gw/reflect_macro.hpp"

namespace {

struct Widget {
    int32_t count;
    float scale;
    bool active;
};

} // namespace

ENGINE_REFLECT(Widget, count, scale, active)

TEST(Reflection, RegisteredTypeIsFindable) {
    const gw::TypeInfo* info = gw::ReflectionRegistry::instance().find("Widget");
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->size, sizeof(Widget));
    EXPECT_EQ(info->align, alignof(Widget));
}

TEST(Reflection, FieldsMatchDeclarationOrderAndOffsets) {
    const gw::TypeInfo* info = gw::ReflectionRegistry::instance().find("Widget");
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->fields.size(), 3u);

    EXPECT_EQ(info->fields[0].name, "count");
    EXPECT_EQ(info->fields[0].type, gw::FieldType::I32);
    EXPECT_EQ(info->fields[0].offset, offsetof(Widget, count));

    EXPECT_EQ(info->fields[1].name, "scale");
    EXPECT_EQ(info->fields[1].type, gw::FieldType::F32);
    EXPECT_EQ(info->fields[1].offset, offsetof(Widget, scale));

    EXPECT_EQ(info->fields[2].name, "active");
    EXPECT_EQ(info->fields[2].type, gw::FieldType::Bool);
    EXPECT_EQ(info->fields[2].offset, offsetof(Widget, active));
}

TEST(Reflection, UnknownTypeReturnsNull) {
    EXPECT_EQ(gw::ReflectionRegistry::instance().find("DoesNotExist"), nullptr);
}
