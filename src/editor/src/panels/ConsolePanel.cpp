#include "gw/editor/panels/ConsolePanel.hpp"

namespace gw::editor {

void ConsolePanel::log(std::string line) { _lines.push_back(std::move(line)); }

void ConsolePanel::clear() { _lines.clear(); }

PanelView ConsolePanel::view() const {
    PanelView view;
    view.title = "Console";
    for (const auto& line : _lines) {
        view.rows.push_back(PanelRow{line, RowStyle::Normal, 0, {}});
    }
    return view;
}

} // namespace gw::editor
