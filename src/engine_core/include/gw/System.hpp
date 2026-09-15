#pragma once
#include <string_view>
#include <vector>

namespace gw {

class World;

enum class Phase { Init, FixedUpdate, Update, Render };

using SystemFn = void (*)(World&);

struct After {
    std::string_view name;
};

struct Before {
    std::string_view name;
};

struct SystemInfo {
    std::string_view name;
    SystemFn fn;
    Phase phase;
    std::vector<std::string_view> after;
    std::vector<std::string_view> before;
};

namespace detail {

inline void apply_constraint(SystemInfo& info, After a) { info.after.push_back(a.name); }
inline void apply_constraint(SystemInfo& info, Before b) { info.before.push_back(b.name); }

template <typename... Constraints>
SystemInfo make_system_info(std::string_view name, SystemFn fn, Phase phase, Constraints... cs) {
    SystemInfo info{name, fn, phase, {}, {}};
    (apply_constraint(info, cs), ...);
    return info;
}

} // namespace detail

class SystemRegistry {
public:
    static SystemRegistry& instance();

    void registerSystem(SystemInfo info);

    /**
     * @brief Returns the systems of @p phase in execution order.
     *
     * Order honors every `after`/`before` constraint. Independent systems keep
     * registration order. Constraints naming a system absent from @p phase are
     * ignored.
     *
     * @throws std::runtime_error if the constraints form a cycle (including a
     *         system referencing itself). The message lists the systems involved.
     */
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

#define ENGINE_SYSTEM(fn, phase, ...) \
    namespace { \
    const ::gw::SystemRegistrar GW_SYS_CAT(gw_system_registrar_, __COUNTER__){ \
        ::gw::detail::make_system_info(#fn, &fn, phase __VA_OPT__(,) __VA_ARGS__) \
    }; \
    }
