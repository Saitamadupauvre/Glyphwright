#pragma once
#include <cstdint>

namespace gw {

struct Entity {
    uint32_t id = 0;
    uint32_t generation = 0;

    bool operator==(const Entity&) const = default;
};

constexpr Entity kInvalidEntity{0, 0};

} // namespace gw
