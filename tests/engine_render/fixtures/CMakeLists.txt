add_library(gw_test_dltestlib MODULE DLTestLib.cpp)
target_compile_features(gw_test_dltestlib PUBLIC cxx_std_20)
set_target_properties(gw_test_dltestlib PROPERTIES PREFIX "")

target_compile_definitions(gw_tests PRIVATE
    GW_TEST_DLTESTLIB_PATH="$<TARGET_FILE:gw_test_dltestlib>"
)
add_dependencies(gw_tests gw_test_dltestlib)
