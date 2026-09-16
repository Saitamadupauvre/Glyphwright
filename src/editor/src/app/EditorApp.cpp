#include "gw/editor/app/EditorApp.hpp"
#include <algorithm>
#include <string_view>
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
                      std::vector<KnownType> knownTypes, std::filesystem::path projectRoot,
                      std::vector<KnownType> knownSingletonTypes,
                      std::filesystem::path rendererLibraryPath)
    : _world(world),
      _entityListPanel(world),
      _propertiesPanel(world, registry, std::move(knownTypes), std::move(knownSingletonTypes)),
      _folderPanel(std::move(projectRoot)) {
    if (!rendererLibraryPath.empty()) {
        _rendererLoader.load(rendererLibraryPath, _viewportRect);
    }
}

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
        const RendererRect rect{0, 0, static_cast<uint32_t>(std::max(0, _viewportBox.x_max - _viewportBox.x_min + 1)),
                                static_cast<uint32_t>(std::max(0, _viewportBox.y_max - _viewportBox.y_min + 1))};
        if (rect.width != _viewportRect.width || rect.height != _viewportRect.height) {
            _viewportRect = rect;
            _rendererLoader.setRect(rect);
        }

        if (rect.width > 0 && rect.height > 0) {
            std::vector<DrawCommand> commands;
            _world.each<Transform>([&](Entity, Transform& transform) {
                const float cellX = transform.x + static_cast<float>(rect.width) / 2.0f;
                const float cellY = transform.y + static_cast<float>(rect.height) / 2.0f;
                commands.push_back(DrawCommand{DrawCommandKind::FilledRect, cellX, cellY,
                                               transform.scale_x, transform.scale_y,
                                               200, 200, 200, 255});
            });
            _rendererLoader.beginFrame();
            _rendererLoader.drawBatch(commands.data(), commands.size());
            _rendererLoader.endFrame();
        }

        const char* frameText = _rendererLoader.frameText();
        return ftxui::window(
                   ftxui::text(" Game ") | ftxui::bold,
                   _viewportPanel.render(frameText ? frameText : std::string_view{}) | ftxui::flex |
                       ftxui::reflect(_viewportBox)) |
               ftxui::flex;
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
