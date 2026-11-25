#include <gtest/gtest.h>
#include "gw/editor/panels/ViewportPanel.hpp"

TEST(ViewportPanel, EmptyFrameTextProducesNoRows) {
    auto rows = gw::editor::ViewportPanel::parseFrame("");
    EXPECT_TRUE(rows.empty());
}

TEST(ViewportPanel, PlainTextProducesUncoloredCells) {
    auto rows = gw::editor::ViewportPanel::parseFrame("hi");
    ASSERT_EQ(rows.size(), 1u);
    ASSERT_EQ(rows[0].size(), 2u);
    EXPECT_EQ(rows[0][0].character, "h");
    EXPECT_FALSE(rows[0][0].hasColor);
    EXPECT_EQ(rows[0][1].character, "i");
}

TEST(ViewportPanel, NewlineStartsNewRow) {
    auto rows = gw::editor::ViewportPanel::parseFrame("ab\ncd");
    ASSERT_EQ(rows.size(), 2u);
    ASSERT_EQ(rows[0].size(), 2u);
    ASSERT_EQ(rows[1].size(), 2u);
    EXPECT_EQ(rows[0][0].character, "a");
    EXPECT_EQ(rows[1][1].character, "d");
}

TEST(ViewportPanel, TrailingNewlineProducesTrailingEmptyRow) {
    auto rows = gw::editor::ViewportPanel::parseFrame("a\n");
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0].size(), 1u);
    EXPECT_TRUE(rows[1].empty());
}

TEST(ViewportPanel, TruecolorEscapeAppliesColorToFollowingCells) {
    auto rows = gw::editor::ViewportPanel::parseFrame("\x1b[38;2;10;20;30mx");
    ASSERT_EQ(rows.size(), 1u);
    ASSERT_EQ(rows[0].size(), 1u);
    EXPECT_TRUE(rows[0][0].hasColor);
    EXPECT_EQ(rows[0][0].r, 10);
    EXPECT_EQ(rows[0][0].g, 20);
    EXPECT_EQ(rows[0][0].b, 30);
}

TEST(ViewportPanel, ResetEscapeClearsColor) {
    auto rows = gw::editor::ViewportPanel::parseFrame("\x1b[38;2;10;20;30mx\x1b[0my");
    ASSERT_EQ(rows.size(), 1u);
    ASSERT_EQ(rows[0].size(), 2u);
    EXPECT_TRUE(rows[0][0].hasColor);
    EXPECT_FALSE(rows[0][1].hasColor);
}

TEST(ViewportPanel, MultibyteUtf8CharacterKeptAsSingleCell) {
    auto rows = gw::editor::ViewportPanel::parseFrame("\xe2\x96\x80");
    ASSERT_EQ(rows.size(), 1u);
    ASSERT_EQ(rows[0].size(), 1u);
    EXPECT_EQ(rows[0][0].character, "\xe2\x96\x80");
}

TEST(ViewportPanel, RenderProducesNonNullElement) {
    gw::editor::ViewportPanel panel;
    auto element = panel.render("hi");
    EXPECT_NE(element, nullptr);
}
