#pragma once
#include <string_view>
#include <vector>

namespace gw {

class World;

enum class Phase { Init, FixedUpdate, Update, Render };

using SystemFn = void (*)(World&);

struct SystemInfo {
    std::string_view name;
    SystemFn fn;
    Phase phase;
    std::vector<std::string_view> after;
    std::vector<std::string_view> before;
};

class SystemRegistry {
public:
    static SystemRegistry& instance();

    void registerSystem(SystemInfo info);
    std::vector<SystemInfo> resolveOrder(Phase phase) const;

private:
    std::vector<SystemInfo> _systems;
};

struct SystemRegistrar {
    explicit SystemRegistrar(SystemInfo info) {
        SystemRegistry::instance().registerSystem(std::move(info));
    }
};

} // namespace gw

#define GW_SYS_CAT(a, b) GW_SYS_CAT_(a, b)
#define GW_SYS_CAT_(a, b) a##b

#define ENGINE_SYSTEM(fn, phase) \
    namespace { \
    const ::gw::SystemRegistrar GW_SYS_CAT(gw_system_registrar_, __COUNTER__){ \
        ::gw::SystemInfo{#fn, &fn, phase, {}, {}} \
    }; \
    }
