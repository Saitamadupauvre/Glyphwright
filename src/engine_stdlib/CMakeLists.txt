add_library(engine_stdlib STATIC
    src/components.cpp
)

target_include_directories(engine_stdlib PUBLIC include)
target_link_libraries(engine_stdlib PUBLIC engine_core)
target_compile_features(engine_stdlib PUBLIC cxx_std_20)
