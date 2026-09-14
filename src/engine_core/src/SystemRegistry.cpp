#include "gw/System.hpp"
#include <functional>

namespace gw {

SystemRegistry& SystemRegistry::instance() {
    static SystemRegistry registry;
    return registry;
}

void SystemRegistry::registerSystem(SystemInfo info) {
    _systems.push_back(std::move(info));
}

std::vector<SystemInfo> SystemRegistry::resolveOrder(Phase phase) const {
    std::vector<SystemInfo> phaseSystems;
    for (const auto& s : _systems) {
        if (s.phase == phase) phaseSystems.push_back(s);
    }

    auto findByName = [&](std::string_view name) -> int {
        for (size_t i = 0; i < phaseSystems.size(); ++i) {
            if (phaseSystems[i].name == name) return static_cast<int>(i);
        }
        return -1;
    };

    for (size_t i = 0; i < phaseSystems.size(); ++i) {
        for (auto dep : phaseSystems[i].before) {
            int idx = findByName(dep);
            if (idx >= 0) phaseSystems[static_cast<size_t>(idx)].after.push_back(phaseSystems[i].name);
        }
    }

    std::vector<SystemInfo> sorted;
    std::vector<bool> placed(phaseSystems.size(), false);

    std::function<void(size_t)> visit = [&](size_t i) {
        if (placed[i]) return;
        placed[i] = true;
        for (auto dep : phaseSystems[i].after) {
            int idx = findByName(dep);
            if (idx >= 0) visit(static_cast<size_t>(idx));
        }
        sorted.push_back(phaseSystems[i]);
    };

    for (size_t i = 0; i < phaseSystems.size(); ++i) {
        visit(i);
    }

    return sorted;
}

} // namespace gw
