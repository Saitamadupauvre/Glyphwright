#include "gw/editor/panels/ViewportPanel.hpp"
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

namespace gw::editor {

namespace {

size_t utf8Length(unsigned char lead) {
    if ((lead & 0x80) == 0x00) return 1;
    if ((lead & 0xE0) == 0xC0) return 2;
    if ((lead & 0xF0) == 0xE0) return 3;
    if ((lead & 0xF8) == 0xF0) return 4;
    return 1;
}

class ViewportFrameNode : public ftxui::Node {
public:
    explicit ViewportFrameNode(std::vector<std::vector<ViewportCell>> rows) : _rows(std::move(rows)) {}

    void ComputeRequirement() override {
        requirement_.min_x = 0;
        requirement_.min_y = 0;
        requirement_.flex_grow_x = 1;
        requirement_.flex_grow_y = 1;
    }

    void Render(ftxui::Screen& screen) override {
        const int width = box_.x_max - box_.x_min + 1;
        const int height = box_.y_max - box_.y_min + 1;
        if (width <= 0 || height <= 0) return;

        for (int y = 0; y < height && y < static_cast<int>(_rows.size()); ++y) {
            const auto& row = _rows[y];
            for (int x = 0; x < width && x < static_cast<int>(row.size()); ++x) {
                const ViewportCell& cell = row[x];
                ftxui::Cell& screenCell = screen.PixelAt(box_.x_min + x, box_.y_min + y);
                screenCell.character = cell.character;
                if (cell.hasColor) {
                    screenCell.foreground_color = ftxui::Color(cell.r, cell.g, cell.b);
                }
            }
        }
    }

private:
    std::vector<std::vector<ViewportCell>> _rows;
};

} // namespace

std::vector<std::vector<ViewportCell>> ViewportPanel::parseFrame(std::string_view frameText) {
    std::vector<std::vector<ViewportCell>> rows;
    if (frameText.empty()) return rows;

    rows.emplace_back();
    bool hasColor = false;
    uint8_t r = 0, g = 0, b = 0;

    size_t i = 0;
    while (i < frameText.size()) {
        const unsigned char c = static_cast<unsigned char>(frameText[i]);

        if (c == '\n') {
            rows.emplace_back();
            ++i;
            continue;
        }

        if (c == 0x1b && i + 1 < frameText.size() && frameText[i + 1] == '[') {
            const size_t end = frameText.find('m', i + 2);
            if (end == std::string_view::npos) break;
            const std::string_view params = frameText.substr(i + 2, end - (i + 2));

            std::vector<int> values;
            size_t start = 0;
            while (start <= params.size()) {
                const size_t sep = params.find(';', start);
                const std::string_view token = params.substr(start, sep == std::string_view::npos ? std::string_view::npos : sep - start);
                values.push_back(token.empty() ? 0 : std::stoi(std::string(token)));
                if (sep == std::string_view::npos) break;
                start = sep + 1;
            }

            for (size_t v = 0; v < values.size(); ++v) {
                if (values[v] == 0) {
                    hasColor = false;
                } else if (values[v] == 38 && v + 4 < values.size() && values[v + 1] == 2) {
                    hasColor = true;
                    r = static_cast<uint8_t>(values[v + 2]);
                    g = static_cast<uint8_t>(values[v + 3]);
                    b = static_cast<uint8_t>(values[v + 4]);
                    v += 4;
                } else if (values[v] == 39) {
                    hasColor = false;
                }
            }

            i = end + 1;
            continue;
        }

        const size_t length = utf8Length(c);
        ViewportCell cell;
        cell.character = std::string(frameText.substr(i, std::min(length, frameText.size() - i)));
        cell.hasColor = hasColor;
        cell.r = r;
        cell.g = g;
        cell.b = b;
        rows.back().push_back(std::move(cell));
        i += length;
    }

    return rows;
}

ftxui::Element ViewportPanel::render(std::string_view frameText) const {
    return std::make_shared<ViewportFrameNode>(parseFrame(frameText));
}

} // namespace gw::editor
