#pragma once
#include <vector>
#include "gw/System.hpp"

namespace gw {

class World;

class GameLoop {
public:
    enum class State { Stopped, Playing, Paused };

    explicit GameLoop(World& world, SystemRegistry& registry = SystemRegistry::instance(),
                       float fixedDt = 1.0f / 60.0f, float maxFrameTime = 0.25f);

    void play();
    void stop();
    void pause();
    void resume();
    void tick(float dt);

    State state() const { return _state; }

private:
    void runPhase(const std::vector<SystemInfo>& systems);

    World* _world;
    SystemRegistry* _registry;
    float _fixedDt;
    float _maxFrameTime;
    float _accumulator = 0.0f;
    State _state = State::Stopped;

    std::vector<SystemInfo> _initSystems;
    std::vector<SystemInfo> _fixedUpdateSystems;
    std::vector<SystemInfo> _updateSystems;
    std::vector<SystemInfo> _renderSystems;
};

} // namespace gw
