#include <gtest/gtest.h>
#include <fstream>
#include "gw/editor/panels/FolderPanel.hpp"

namespace {

namespace fs = std::filesystem;

class FolderPanelTest : public ::testing::Test {
protected:
    void SetUp() override {
        _root = fs::temp_directory_path() / fs::path("gw_folder_panel_test");
        fs::remove_all(_root);
        fs::create_directories(_root);
    }

    void TearDown() override { fs::remove_all(_root); }

    fs::path _root;
};

} // namespace

TEST_F(FolderPanelTest, EmptyDirectoryYieldsEmptyRows) {
    gw::editor::FolderPanel panel(_root);
    EXPECT_TRUE(panel.view().rows.empty());
}

TEST_F(FolderPanelTest, MissingRootYieldsEmptyRows) {
    gw::editor::FolderPanel panel(_root / "does_not_exist");
    EXPECT_TRUE(panel.view().rows.empty());
}

TEST_F(FolderPanelTest, DirectoriesSortBeforeFilesAlphabetically) {
    std::ofstream(_root / "b.txt").put('x');
    std::ofstream(_root / "a.txt").put('x');
    fs::create_directory(_root / "zdir");
    fs::create_directory(_root / "adir");

    gw::editor::FolderPanel panel(_root);
    auto rows = panel.view().rows;
    ASSERT_EQ(rows.size(), 4u);
    EXPECT_EQ(rows[0].text, "adir");
    EXPECT_EQ(rows[1].text, "zdir");
    EXPECT_EQ(rows[2].text, "a.txt");
    EXPECT_EQ(rows[3].text, "b.txt");
}

TEST_F(FolderPanelTest, NestedSubdirectoryHasCorrectDepth) {
    fs::create_directory(_root / "sub");
    std::ofstream(_root / "sub" / "nested.txt").put('x');

    gw::editor::FolderPanel panel(_root);
    auto rows = panel.view().rows;
    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].text, "sub");
    EXPECT_EQ(rows[0].depth, 0);
    ASSERT_EQ(rows[0].children.size(), 1u);
    EXPECT_EQ(rows[0].children[0].text, "nested.txt");
    EXPECT_EQ(rows[0].children[0].depth, 1);
}
