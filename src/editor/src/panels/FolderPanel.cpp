#include "gw/editor/panels/FolderPanel.hpp"
#include <algorithm>
#include <system_error>

namespace gw::editor {

namespace fs = std::filesystem;

FolderPanel::FolderPanel(fs::path root) : _root(std::move(root)) {}

const fs::path& FolderPanel::root() const { return _root; }

PanelRow FolderPanel::buildNode(const fs::path& path, int depth) const {
    PanelRow row{path.filename().string(), RowStyle::Normal, depth, {}};

    std::error_code ec;
    if (!fs::is_directory(path, ec) || ec) return row;

    std::vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(path, ec)) {
        entries.push_back(entry.path());
    }
    if (ec) return row;

    std::sort(entries.begin(), entries.end(), [](const fs::path& a, const fs::path& b) {
        bool aDir = fs::is_directory(a);
        bool bDir = fs::is_directory(b);
        if (aDir != bDir) return aDir;
        return a.filename().string() < b.filename().string();
    });

    for (const auto& entry : entries) {
        row.children.push_back(buildNode(entry, depth + 1));
    }
    return row;
}

PanelView FolderPanel::view() const {
    PanelView view;
    view.title = "Folder";

    std::error_code ec;
    if (!fs::exists(_root, ec) || ec) return view;

    if (fs::is_directory(_root, ec) && !ec) {
        view.rows = buildNode(_root, -1).children;
    } else {
        view.rows.push_back(PanelRow{_root.filename().string(), RowStyle::Normal, 0, {}});
    }
    return view;
}

} // namespace gw::editor
