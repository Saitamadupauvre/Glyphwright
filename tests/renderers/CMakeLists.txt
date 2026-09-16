target_sources(gw_tests PRIVATE
    test_DrawCommand.cpp
    test_RendererExportMacro.cpp
    test_RendererLoader.cpp
)

if(GW_BUILD_RENDERER_CHAFA)
    target_sources(gw_tests PRIVATE
        test_ChafaRenderer.cpp
        ${CMAKE_SOURCE_DIR}/renderers/chafa/src/ChafaRenderer.cpp
    )
    target_include_directories(gw_tests PRIVATE ${CMAKE_SOURCE_DIR}/renderers/chafa/src)
    target_link_libraries(gw_tests PRIVATE chafa_vendored)
endif()

add_subdirectory(fixtures)
