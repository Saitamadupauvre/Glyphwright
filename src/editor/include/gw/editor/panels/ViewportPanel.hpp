#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <ftxui/dom/elements.hpp>

namespace gw::editor {

struct ViewportCell {
    std::string character = " ";
    bool hasColor = false;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

class ViewportPanel {
public:
    static std::vector<std::vector<ViewportCell>> parseFrame(std::string_view frameText);

    ftxui::Element render(std::string_view frameText) const;
};

} // namespace gw::editor
