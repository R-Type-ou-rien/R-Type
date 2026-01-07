#include "ServerGameEngine.hpp"
#include <chrono>
#include "Context.hpp"
#include "GameEngineBase.hpp"

ServerGameEngine::ServerGameEngine() {}

int ServerGameEngine::init() {
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<SpawnSystem>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<ComponentSenderSystem>();
    _ecs.systems.addSystem<PhysicsSystem>();
    _ecs.systems.addSystem<InputSystem>(input_manager);

    return SUCCESS;
}

int ServerGameEngine::run() {
    system_context ctx = {0, _texture_manager, _sound_manager, _music_manager, input_manager};
    auto last_time = std::chrono::high_resolution_clock::now();
    Environment env(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::SERVER);

    init();
    if (_init_function)
        _init_function(env, input_manager);

    while (1) {
        auto now = std::chrono::high_resolution_clock::now();
        ctx.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;
        if (_loop_function)
            _loop_function(env, input_manager);
        _ecs.update(ctx);
    }
    return SUCCESS;
}
