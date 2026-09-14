#pragma once
#include <filesystem>
#include "gw/editor/PanelView.hpp"

namespace gw::editor {

class FolderPanel {
public:
    explicit FolderPanel(std::filesystem::path root);

    PanelView view() const;
    const std::filesystem::path& root() const;

private:
    PanelRow buildNode(const std::filesystem::path& path, int depth) const;

    std::filesystem::path _root;
};

} // namespace gw::editor
