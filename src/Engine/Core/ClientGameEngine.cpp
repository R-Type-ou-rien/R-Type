#include "ClientGameEngine.hpp"
#include "CollisionSystem.hpp"
#include "ActionScriptSystem.hpp"
#include "Components/StandardComponents.hpp"
#include "PatternSystem/PatternSystem.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name) {}

int ClientGameEngine::init() {
    InputManager input_manager;

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<SpawnSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<InputSystem>(_ecs.input);
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();

    if (_init_function)
        _init_function(_ecs);

    // if mode local
    _ecs.systems.addSystem<PhysicsSystem>();
    return 0;
}

void ClientGameEngine::setUserFunction(std::function<void(ECS& ecs)> user_function) {
    _function = user_function;
    return;
}

void ClientGameEngine::setInitFunction(std::function<void(ECS& ecs)> user_function) {
    _init_function = user_function;
    return;
}

void ClientGameEngine::handleEvent() {
    while (std::optional<sf::Event> event = _window_manager.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window_manager.getWindow().close();
        if (event->is<sf::Event::FocusLost>())
            _ecs.input.setWindowHasFocus(false);
        if (event->is<sf::Event::FocusGained>())
            _ecs.input.setWindowHasFocus(true);
    }
}

int ClientGameEngine::run() {
    sf::Clock clock;
    system_context context = {0, _ecs._textureManager, _window_manager.getWindow(), _ecs.input};

    this->init();
    while (_window_manager.isOpen()) {
        context.dt = clock.restart().asSeconds();
        handleEvent();
        _window_manager.clear();
        if (_function)
            _function(_ecs);
        _ecs.update(context);
        _window_manager.display();
    }
    return 0;
}
