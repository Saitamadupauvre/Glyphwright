#include <gtest/gtest.h>
#include "gw/editor/panels/ViewportPanel.hpp"

TEST(ViewportPanel, ViewHasTitleAndNoRows) {
    gw::editor::ViewportPanel panel;
    auto view = panel.view();
    EXPECT_EQ(view.title, "Game");
    EXPECT_TRUE(view.rows.empty());
}
