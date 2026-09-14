add_library(engine_render STATIC
    src/RendererLoader.cpp
)

target_include_directories(engine_render PUBLIC include)
target_compile_features(engine_render PUBLIC cxx_std_20)
target_link_libraries(engine_render PUBLIC ${CMAKE_DL_LIBS})
