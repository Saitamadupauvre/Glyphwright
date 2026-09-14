#include "gw/editor/app/EditorApp.hpp"
#include <ftxui/component/screen_interactive.hpp>

namespace gw::editor {

namespace {

ftxui::Element decorate(const PanelRow& row) {
    auto element = ftxui::text(std::string(row.depth * 2, ' ') + row.text);
    switch (row.style) {
        case RowStyle::Bold: return element | ftxui::bold;
        case RowStyle::Dim: return element | ftxui::dim;
        case RowStyle::Normal: return element;
    }
    return element;
}

} // namespace

EditorApp::EditorApp(World& world, const ReflectionRegistry& registry,
                      std::vector<KnownType> knownTypes, std::filesystem::path projectRoot)
    : _world(world),
      _entityListPanel(world),
      _propertiesPanel(world, registry, std::move(knownTypes)),
      _folderPanel(std::move(projectRoot)) {}

ftxui::Element EditorApp::renderRows(const std::vector<PanelRow>& rows) const {
    ftxui::Elements lines;
    for (const auto& row : rows) {
        lines.push_back(decorate(row));
        if (!row.children.empty()) lines.push_back(renderRows(row.children));
    }
    return ftxui::vbox(std::move(lines));
}

ftxui::Element EditorApp::renderPanel(const PanelView& view) const {
    return ftxui::window(ftxui::text(" " + view.title + " ") | ftxui::bold,
                         renderRows(view.rows) | ftxui::vscroll_indicator | ftxui::frame | ftxui::flex);
}

ftxui::Component EditorApp::buildEntityListComponent() {
    _entityLabels.clear();
    for (const auto& row : _entityListPanel.view().rows) _entityLabels.push_back(row.text);
    auto menu = ftxui::Menu(&_entityLabels, &_selectedEntityIndex);
    return ftxui::Renderer(menu, [this, menu] {
        return ftxui::window(ftxui::text(" " + _entityListPanel.view().title + " ") | ftxui::bold,
                             menu->Render() | ftxui::vscroll_indicator | ftxui::frame | ftxui::flex);
    });
}

ftxui::Component EditorApp::buildViewportComponent() {
    return ftxui::Renderer([this] {
        auto view = _viewportPanel.view();
        return ftxui::window(ftxui::text(" " + view.title + " ") | ftxui::bold,
                             ftxui::filler()) | ftxui::flex;
    });
}

ftxui::Component EditorApp::buildPropertiesComponent() {
    return ftxui::Renderer([this] {
        std::optional<Entity> selected;
        if (_selectedEntityIndex >= 0 &&
            static_cast<std::size_t>(_selectedEntityIndex) < _entityListPanel.entityCount()) {
            selected = _entityListPanel.entityAt(_selectedEntityIndex);
        }
        return renderPanel(_propertiesPanel.view(selected));
    });
}

ftxui::Component EditorApp::buildBottomComponent() {
    auto tabs = ftxui::Toggle(&_bottomTabLabels, &_selectedBottomTab);

    auto console = ftxui::Renderer([this] { return renderRows(_consolePanel.view().rows); });
    auto project = ftxui::Renderer([this] { return renderRows(_folderPanel.view().rows); });
    auto content = ftxui::Container::Tab({console, project}, &_selectedBottomTab);

    auto container = ftxui::Container::Vertical({tabs, content});
    return ftxui::Renderer(container, [tabs, content] {
        return ftxui::vbox({
                   tabs->Render(),
                   ftxui::separator(),
                   content->Render() | ftxui::vscroll_indicator | ftxui::frame | ftxui::flex,
               }) |
               ftxui::border;
    });
}

void EditorApp::run() {
    auto entityList = buildEntityListComponent();
    auto viewport = buildViewportComponent();
    auto properties = buildPropertiesComponent();
    auto bottom = buildBottomComponent();

    auto split = [](ftxui::Component main, ftxui::Component back, ftxui::Direction direction, int* size) {
        return ftxui::ResizableSplit({
            .main = std::move(main),
            .back = std::move(back),
            .direction = direction,
            .main_size = size,
            .separator_func = [] { return ftxui::separatorCharacter(" "); },
        });
    };

    auto centerRight = split(properties, viewport, ftxui::Direction::Right, &_rightPaneWidth);
    auto top = split(entityList, centerRight, ftxui::Direction::Left, &_leftPaneWidth);
    auto layout = split(bottom, top, ftxui::Direction::Down, &_bottomPaneHeight);

    auto screen = ftxui::ScreenInteractive::Fullscreen();
    screen.Loop(layout);
}

} // namespace gw::editor
