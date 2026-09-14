#include <gtest/gtest.h>
#include "gw/DLLoader.hpp"

namespace {

using EntryFn = int (*)();

TEST(DLLoader, OpenNonexistentPathFails) {
    gw::DLLoader<EntryFn> loader;
    EXPECT_FALSE(loader.open("/nonexistent/path/does_not_exist.so", "dl_test_entry_point"));
    EXPECT_EQ(loader.lastError(), gw::DLLoadError::FileNotFound);
    EXPECT_FALSE(loader.isOpen());
}

TEST(DLLoader, OpenMissingSymbolFails) {
    gw::DLLoader<EntryFn> loader;
    EXPECT_FALSE(loader.open(GW_TEST_DLTESTLIB_PATH, "symbol_that_does_not_exist"));
    EXPECT_EQ(loader.lastError(), gw::DLLoadError::EntryPointNotFound);
    EXPECT_FALSE(loader.isOpen());
}

TEST(DLLoader, OpenGoodLibrarySucceeds) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    EXPECT_TRUE(loader.isOpen());
    EXPECT_EQ(loader.lastError(), gw::DLLoadError::None);
    ASSERT_NE(loader.entryPoint(), nullptr);
    EXPECT_EQ(loader.entryPoint()(), 42);
    EXPECT_EQ(loader.path(), std::filesystem::path(GW_TEST_DLTESTLIB_PATH));
}

TEST(DLLoader, ReopenClosesPreviousHandle) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    EXPECT_FALSE(loader.open("/nonexistent/path/does_not_exist.so", "dl_test_entry_point"));
    EXPECT_FALSE(loader.isOpen());

    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    EXPECT_TRUE(loader.isOpen());
    EXPECT_EQ(loader.entryPoint()(), 42);
}

TEST(DLLoader, MoveConstructTransfersOwnership) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));

    gw::DLLoader<EntryFn> moved(std::move(loader));
    EXPECT_TRUE(moved.isOpen());
    EXPECT_EQ(moved.entryPoint()(), 42);
    EXPECT_FALSE(loader.isOpen());
    EXPECT_EQ(loader.entryPoint(), nullptr);
}

TEST(DLLoader, MoveAssignTransfersOwnership) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));

    gw::DLLoader<EntryFn> other;
    other = std::move(loader);
    EXPECT_TRUE(other.isOpen());
    EXPECT_EQ(other.entryPoint()(), 42);
    EXPECT_FALSE(loader.isOpen());
}

TEST(DLLoader, MoveAssignClosesTargetsExistingHandle) {
    gw::DLLoader<EntryFn> a;
    ASSERT_TRUE(a.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));

    gw::DLLoader<EntryFn> b;
    ASSERT_TRUE(b.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));

    b = std::move(a);
    EXPECT_TRUE(b.isOpen());
    EXPECT_FALSE(a.isOpen());
}

TEST(DLLoader, DestructorClosesOpenHandle) {
    {
        gw::DLLoader<EntryFn> loader;
        ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    }
    SUCCEED();
}

TEST(DLLoader, SymbolLookupOnUnopenedLoaderReturnsNull) {
    gw::DLLoader<EntryFn> loader;
    EXPECT_EQ((loader.symbol<EntryFn>("dl_test_other_symbol")), nullptr);
}

TEST(DLLoader, SymbolLookupNonexistentReturnsNull) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    EXPECT_EQ((loader.symbol<EntryFn>("nonexistent")), nullptr);
}

TEST(DLLoader, SymbolLookupAdditionalSymbolSucceeds) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    auto fn = loader.symbol<EntryFn>("dl_test_other_symbol");
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn(), 7);
}

TEST(DLLoader, DoubleCloseIsNoop) {
    gw::DLLoader<EntryFn> loader;
    ASSERT_TRUE(loader.open(GW_TEST_DLTESTLIB_PATH, "dl_test_entry_point"));
    loader.close();
    EXPECT_FALSE(loader.isOpen());
    loader.close();
    EXPECT_FALSE(loader.isOpen());
}

} // namespace
