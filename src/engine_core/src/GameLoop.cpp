#include "gw/GameLoop.hpp"
#include "gw/World.hpp"

namespace gw {

GameLoop::GameLoop(World& world, SystemRegistry& registry, float fixedDt, float maxFrameTime)
    : _world(&world), _registry(&registry), _fixedDt(fixedDt), _maxFrameTime(maxFrameTime) {}

void GameLoop::play() {
    if (_state != State::Stopped) return;

    _initSystems = _registry->resolveOrder(Phase::Init);
    _fixedUpdateSystems = _registry->resolveOrder(Phase::FixedUpdate);
    _updateSystems = _registry->resolveOrder(Phase::Update);
    _renderSystems = _registry->resolveOrder(Phase::Render);

    _accumulator = 0.0f;
    _state = State::Playing;
    runPhase(_initSystems);
}

void GameLoop::stop() {
    _state = State::Stopped;
    _accumulator = 0.0f;
    _initSystems.clear();
    _fixedUpdateSystems.clear();
    _updateSystems.clear();
    _renderSystems.clear();
}

void GameLoop::pause() {
    if (_state == State::Playing) _state = State::Paused;
}

void GameLoop::resume() {
    if (_state == State::Paused) _state = State::Playing;
}

void GameLoop::tick(float dt) {
    if (_state != State::Playing) return;

    if (dt > _maxFrameTime) dt = _maxFrameTime;
    if (dt < 0.0f) dt = 0.0f;

    _accumulator += dt;
    if (_fixedDt > 0.0f) {
        while (_accumulator >= _fixedDt) {
            runPhase(_fixedUpdateSystems);
            _accumulator -= _fixedDt;
        }
    }

    runPhase(_updateSystems);
    runPhase(_renderSystems);
}

void GameLoop::runPhase(const std::vector<SystemInfo>& systems) {
    for (const auto& system : systems) {
        system.fn(*_world);
    }
}

} // namespace gw
