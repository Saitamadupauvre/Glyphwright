include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(gw_tests)

target_link_libraries(gw_tests PRIVATE
    $<LINK_LIBRARY:WHOLE_ARCHIVE,engine_core>
    $<LINK_LIBRARY:WHOLE_ARCHIVE,engine_stdlib>
    engine_render
    GTest::gtest_main
)

add_subdirectory(engine_core)
add_subdirectory(engine_render)
add_subdirectory(engine_stdlib)
add_subdirectory(renderers)

if(GW_BUILD_EDITOR)
    target_link_libraries(gw_tests PRIVATE gw_editor_models)
    add_subdirectory(editor)
endif()

include(GoogleTest)
gtest_discover_tests(gw_tests)
