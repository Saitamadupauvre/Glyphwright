#pragma once
#include <string>
#include <vector>
#include "gw/editor/PanelView.hpp"

namespace gw::editor {

class ConsolePanel {
public:
    void log(std::string line);
    void clear();

    PanelView view() const;

private:
    std::vector<std::string> _lines;
};

} // namespace gw::editor
