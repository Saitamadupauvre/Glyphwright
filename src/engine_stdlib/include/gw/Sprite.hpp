#pragma once
#include <cstdint>

namespace gw {

struct Sprite {
    uint32_t texture_id = 0;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    int32_t layer = 0;
};

} // namespace gw
