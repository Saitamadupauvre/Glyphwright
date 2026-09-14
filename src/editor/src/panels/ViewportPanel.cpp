#include "gw/editor/panels/ViewportPanel.hpp"

namespace gw::editor {

PanelView ViewportPanel::view() const {
    PanelView view;
    view.title = "Game";
    return view;
}

} // namespace gw::editor
