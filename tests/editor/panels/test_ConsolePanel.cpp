#include <gtest/gtest.h>
#include "gw/editor/panels/ConsolePanel.hpp"

TEST(ConsolePanel, EmptyConsoleYieldsNoRows) {
    gw::editor::ConsolePanel panel;
    auto view = panel.view();
    EXPECT_EQ(view.title, "Console");
    EXPECT_TRUE(view.rows.empty());
}

TEST(ConsolePanel, LoggedLinesAppearInOrder) {
    gw::editor::ConsolePanel panel;
    panel.log("first");
    panel.log("second");

    auto rows = panel.view().rows;
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0].text, "first");
    EXPECT_EQ(rows[1].text, "second");
    EXPECT_EQ(rows[0].depth, 0);
    EXPECT_TRUE(rows[0].children.empty());
}

TEST(ConsolePanel, DuplicateLinesAreKept) {
    gw::editor::ConsolePanel panel;
    panel.log("same");
    panel.log("same");
    EXPECT_EQ(panel.view().rows.size(), 2u);
}

TEST(ConsolePanel, ClearRemovesAllLines) {
    gw::editor::ConsolePanel panel;
    panel.log("line");
    panel.clear();
    EXPECT_TRUE(panel.view().rows.empty());
}

TEST(ConsolePanel, ClearOnEmptyIsNoop) {
    gw::editor::ConsolePanel panel;
    panel.clear();
    EXPECT_TRUE(panel.view().rows.empty());
}
