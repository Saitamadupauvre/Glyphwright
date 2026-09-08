#include <gtest/gtest.h>
#include <type_traits>
#include "gw/DrawCommand.hpp"
#include "gw/RendererVTable.hpp"

static_assert(std::is_standard_layout_v<gw::DrawCommand>);
static_assert(std::is_standard_layout_v<gw::DrawCommandBatch>);
static_assert(std::is_standard_layout_v<gw::RendererRect>);
static_assert(std::is_standard_layout_v<gw::StateBuffer>);

namespace {

TEST(DrawCommand, EmptyBatchIsValid) {
    gw::DrawCommandBatch batch{nullptr, 0};
    EXPECT_EQ(batch.commands, nullptr);
    EXPECT_EQ(batch.count, 0u);
}

TEST(DrawCommand, SingleCommandRoundTrips) {
    gw::DrawCommand cmd{gw::DrawCommandKind::FilledRect, 1.0f, 2.0f, 3.0f, 4.0f, 10, 20, 30, 40};
    EXPECT_EQ(cmd.kind, gw::DrawCommandKind::FilledRect);
    EXPECT_FLOAT_EQ(cmd.x, 1.0f);
    EXPECT_FLOAT_EQ(cmd.y, 2.0f);
    EXPECT_FLOAT_EQ(cmd.w, 3.0f);
    EXPECT_FLOAT_EQ(cmd.h, 4.0f);
    EXPECT_EQ(cmd.r, 10);
    EXPECT_EQ(cmd.g, 20);
    EXPECT_EQ(cmd.b, 30);
    EXPECT_EQ(cmd.a, 40);
}

TEST(DrawCommand, MultiCommandBatchIterates) {
    gw::DrawCommand commands[3] = {
        {gw::DrawCommandKind::FilledRect, 0, 0, 1, 1, 1, 1, 1, 1},
        {gw::DrawCommandKind::FilledRect, 1, 1, 2, 2, 2, 2, 2, 2},
        {gw::DrawCommandKind::FilledRect, 2, 2, 3, 3, 3, 3, 3, 3},
    };
    gw::DrawCommandBatch batch{commands, 3};
    ASSERT_EQ(batch.count, 3u);
    for (size_t i = 0; i < batch.count; ++i) {
        EXPECT_FLOAT_EQ(batch.commands[i].x, static_cast<float>(i));
        EXPECT_EQ(batch.commands[i].r, static_cast<uint8_t>(i + 1));
    }
}

} // namespace
