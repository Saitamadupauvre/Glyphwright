include(cmake/VendorChafa.cmake)

add_library(renderer_chafa MODULE
    src/ChafaRenderer.cpp
)

target_link_libraries(renderer_chafa PRIVATE engine_render chafa_vendored)
target_compile_features(renderer_chafa PUBLIC cxx_std_20)
set_target_properties(renderer_chafa PROPERTIES PREFIX "" OUTPUT_NAME "renderer_chafa")
