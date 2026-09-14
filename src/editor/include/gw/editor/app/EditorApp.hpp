#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "gw/Reflection.hpp"
#include "gw/World.hpp"
#include "gw/editor/panels/ConsolePanel.hpp"
#include "gw/editor/panels/EntityListPanel.hpp"
#include "gw/editor/panels/FolderPanel.hpp"
#include "gw/editor/panels/PropertiesPanel.hpp"
#include "gw/editor/panels/ViewportPanel.hpp"

namespace gw::editor {

class EditorApp {
public:
    EditorApp(World& world, const ReflectionRegistry& registry,
              std::vector<KnownType> knownTypes, std::filesystem::path projectRoot);

    void run();

private:
    ftxui::Component buildEntityListComponent();
    ftxui::Component buildViewportComponent();
    ftxui::Component buildPropertiesComponent();
    ftxui::Component buildBottomComponent();
    ftxui::Element renderPanel(const PanelView& view) const;
    ftxui::Element renderRows(const std::vector<PanelRow>& rows) const;

    World& _world;
    EntityListPanel _entityListPanel;
    ViewportPanel _viewportPanel;
    PropertiesPanel _propertiesPanel;
    ConsolePanel _consolePanel;
    FolderPanel _folderPanel;

    std::vector<std::string> _entityLabels;
    std::vector<std::string> _bottomTabLabels{"Console", "Project"};
    int _selectedEntityIndex = 0;
    int _selectedBottomTab = 0;
    int _leftPaneWidth = 30;
    int _rightPaneWidth = 40;
    int _bottomPaneHeight = 12;
};

} // namespace gw::editor
