#include "client_game_engine.hpp"
#include "ecs/common/ISystem.hpp"
#include "ecs/common/MoveSystem/MoveSystem.hpp"


ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name)
{
}

int ClientGameEngine::init()
{
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<InputSystem>();
    if(_init_function)
        _init_function(_ecs);

    // iwf mode local
    _ecs.systems.addSystem<MoveSystem>();
    return 0;
}

void ClientGameEngine::setUserFunction(std::function<void(ECS& ecs)> user_function)
{
    _function = user_function;
    return;
}

void ClientGameEngine::setInitFunction(std::function<void(ECS& ecs)> user_function)
{
    _init_function = user_function;
    return;
}

int ClientGameEngine::run()
{
    sf::Clock clock;
    std::optional<sf::Event> event;
    system_context context = {
        0,
        _ecs._textureManager,
        _window_manager.getWindow()
    };
    this->init();
    while (_window_manager.isOpen()) {
        _window_manager.handleEvent(event);
        _window_manager.clear();
        if(_function)
            _function(_ecs);
        sf::Time elapsed = clock.restart();
        context.dt = elapsed.asSeconds();
        _ecs.update(context);
        _window_manager.display();
    }
    return 0;
}