target_sources(gw_tests PRIVATE
    test_Entity.cpp
    test_ComponentPool.cpp
    test_World.cpp
    test_Reflection.cpp
    test_SystemRegistry.cpp
    scene/test_SceneBinary.cpp
    scene/test_SceneJson.cpp
)

target_include_directories(gw_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
