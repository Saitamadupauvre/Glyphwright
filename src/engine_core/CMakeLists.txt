add_library(engine_core STATIC
    src/native_components.cpp
    src/GameLoop.cpp
    src/Reflection.cpp
    src/SystemRegistry.cpp
    src/World.cpp
    src/scene/JsonValue.cpp
    src/scene/SceneBinary.cpp
    src/scene/SceneJson.cpp
)

target_include_directories(engine_core PUBLIC include)
target_include_directories(engine_core PRIVATE src/scene)
target_compile_features(engine_core PUBLIC cxx_std_20)
