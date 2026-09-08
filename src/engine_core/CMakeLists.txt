add_library(engine_core STATIC
    src/native_components.cpp
    src/Reflection.cpp
    src/SystemRegistry.cpp
    src/World.cpp
)

target_include_directories(engine_core PUBLIC include)
target_compile_features(engine_core PUBLIC cxx_std_20)
