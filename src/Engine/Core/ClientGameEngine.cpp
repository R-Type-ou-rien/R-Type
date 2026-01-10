#include "ClientGameEngine.hpp"
#include <chrono>
#include <ctime>
#include "CollisionSystem.hpp"
#include "ActionScriptSystem.hpp"
#include "Components/StandardComponents.hpp"
#include "GameEngineBase.hpp"
#include "PatternSystem/PatternSystem.hpp"
#include "SpawnSystem.hpp"
#include "AudioSystem.hpp"
#include "PlayerBoundsSystem.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name) {}

int ClientGameEngine::init() {
    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<AudioSystem>();
    _ecs.systems.addSystem<InputSystem>(input_manager);

    // if mode local or prediction (?)
    _ecs.systems.addSystem<PhysicsSystem>();
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<SpawnSystem>();
    _ecs.systems.addSystem<PlayerBoundsSystem>();
    return 0;
}

void ClientGameEngine::handleEvent() {
    while (std::optional<sf::Event> event = _window_manager.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window_manager.getWindow().close();
        if (event->is<sf::Event::FocusLost>())
            input_manager.setWindowHasFocus(false);
        if (event->is<sf::Event::FocusGained>())
            input_manager.setWindowHasFocus(true);
    }
}

int ClientGameEngine::run() {
    system_context context = {0, _texture_manager, _sound_manager, _music_manager, _window_manager.getWindow(), input_manager};
    auto last_time = std::chrono::high_resolution_clock::now();
    Environment env(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::STANDALONE);

    this->init();
    if (_init_function)
        _init_function(env, input_manager);
    while (_window_manager.isOpen()) {
        auto now = std::chrono::high_resolution_clock::now();
        context.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;
        handleEvent();
        _window_manager.clear();
        if (_loop_function)
            _loop_function(env, input_manager);
        _ecs.update(context);
        _window_manager.display();
    }
    return 0;
}
