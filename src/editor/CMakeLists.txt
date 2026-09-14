include(FetchContent)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v7.0.3
)
FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(clap
    GIT_REPOSITORY https://github.com/lucaspujol/clap.git
    GIT_TAG main
)
FetchContent_MakeAvailable(clap)

add_library(gw_editor_models STATIC
    src/model/EntityListModel.cpp
    src/model/InspectorModel.cpp
    src/panels/EntityListPanel.cpp
    src/panels/PropertiesPanel.cpp
    src/panels/FolderPanel.cpp
    src/panels/ViewportPanel.cpp
    src/panels/ConsolePanel.cpp
)

target_include_directories(gw_editor_models PUBLIC include)
target_link_libraries(gw_editor_models PUBLIC engine_core)
target_compile_features(gw_editor_models PUBLIC cxx_std_20)

add_library(gw_editor_shell STATIC
    src/app/EditorApp.cpp
)

target_include_directories(gw_editor_shell PUBLIC include)
target_link_libraries(gw_editor_shell PUBLIC
    gw_editor_models
    ftxui::screen
    ftxui::dom
    ftxui::component
)
target_compile_features(gw_editor_shell PUBLIC cxx_std_20)

add_library(gw_editor_cli STATIC
    src/cli/cli.cpp
    src/cli/project_locator.cpp
    src/cli/project_scaffold.cpp
)

target_include_directories(gw_editor_cli PUBLIC include)
target_link_libraries(gw_editor_cli PUBLIC clap::clap)
target_compile_features(gw_editor_cli PUBLIC cxx_std_20)

add_executable(editor
    src/main.cpp
)

target_link_libraries(editor PRIVATE
    gw_editor_models
    gw_editor_shell
    gw_editor_cli
    $<LINK_LIBRARY:WHOLE_ARCHIVE,engine_core>
    $<LINK_LIBRARY:WHOLE_ARCHIVE,engine_stdlib>
)

target_compile_features(editor PRIVATE cxx_std_20)
