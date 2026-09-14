#pragma once
#include <string>
#include <vector>

namespace gw::editor {

enum class RowStyle { Normal, Bold, Dim };

struct PanelRow {
    std::string text;
    RowStyle style = RowStyle::Normal;
    int depth = 0;
    std::vector<PanelRow> children;
};

struct PanelView {
    std::string title;
    std::vector<PanelRow> rows;
};

} // namespace gw::editor
