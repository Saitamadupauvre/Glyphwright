add_library(gw_test_renderer_good MODULE TestRendererGood.cpp)
target_include_directories(gw_test_renderer_good PRIVATE ${CMAKE_SOURCE_DIR}/src/engine_render/include)
target_compile_features(gw_test_renderer_good PUBLIC cxx_std_20)
set_target_properties(gw_test_renderer_good PROPERTIES PREFIX "")

add_library(gw_test_renderer_null_field MODULE TestRendererNullField.cpp)
target_include_directories(gw_test_renderer_null_field PRIVATE ${CMAKE_SOURCE_DIR}/src/engine_render/include)
target_compile_features(gw_test_renderer_null_field PUBLIC cxx_std_20)
set_target_properties(gw_test_renderer_null_field PROPERTIES PREFIX "")

add_library(gw_test_renderer_no_symbol MODULE TestRendererNoSymbol.cpp)
target_compile_features(gw_test_renderer_no_symbol PUBLIC cxx_std_20)
set_target_properties(gw_test_renderer_no_symbol PROPERTIES PREFIX "")

target_include_directories(gw_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/engine_render/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(gw_tests PRIVATE
    GW_TEST_RENDERER_GOOD_PATH="$<TARGET_FILE:gw_test_renderer_good>"
    GW_TEST_RENDERER_NULL_FIELD_PATH="$<TARGET_FILE:gw_test_renderer_null_field>"
    GW_TEST_RENDERER_NO_SYMBOL_PATH="$<TARGET_FILE:gw_test_renderer_no_symbol>"
)
add_dependencies(gw_tests gw_test_renderer_good gw_test_renderer_null_field gw_test_renderer_no_symbol)

target_sources(gw_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/TestRendererGood.cpp
)
