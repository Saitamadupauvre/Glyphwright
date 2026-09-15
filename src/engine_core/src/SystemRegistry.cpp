#include "gw/System.hpp"
#include <functional>
#include <stdexcept>
#include <string>

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

    enum class Mark { White, Grey, Black };
    std::vector<Mark> marks(phaseSystems.size(), Mark::White);
    std::vector<size_t> stack;
    std::vector<SystemInfo> sorted;

    std::function<void(size_t)> visit = [&](size_t i) {
        if (marks[i] == Mark::Black) return;
        if (marks[i] == Mark::Grey) {
            std::string msg = "system dependency cycle: ";
            size_t start = 0;
            while (stack[start] != i) ++start;
            for (size_t k = start; k < stack.size(); ++k) {
                msg += std::string(phaseSystems[stack[k]].name);
                msg += " -> ";
            }
            msg += std::string(phaseSystems[i].name);
            throw std::runtime_error(msg);
        }
        marks[i] = Mark::Grey;
        stack.push_back(i);
        for (auto dep : phaseSystems[i].after) {
            int idx = findByName(dep);
            if (idx >= 0) visit(static_cast<size_t>(idx));
        }
        stack.pop_back();
        marks[i] = Mark::Black;
        sorted.push_back(phaseSystems[i]);
    };

    for (size_t i = 0; i < phaseSystems.size(); ++i) {
        visit(i);
    }

    return sorted;
}

} // namespace gw
