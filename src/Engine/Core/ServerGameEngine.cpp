#include "ServerGameEngine.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <ostream>
#include "Network/Network.hpp"

ServerGameEngine::ServerGameEngine() : _network_server(4242, 100) {}

ServerGameEngine::~ServerGameEngine() {}

int ServerGameEngine::init() {
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<ComponentSenderSystem>();
    _ecs.systems.addSystem<PhysicsSystem>();

    if (_init_function)
        _init_function(_ecs);

    _network_server.Start();
    return 0;
}

void ServerGameEngine::setUserFunction(std::function<void(ECS& ecs)> user_function) {
    _function = user_function;
    return;
}

void ServerGameEngine::setInitFunction(std::function<void(ECS& ecs)> user_function) {
    _init_function = user_function;
    return;
}

int ServerGameEngine::run() {
    sf::Clock clock;
    system_context context = {0,
        _ecs._textureManager, 
        std::nullopt, 
        std::nullopt, 
        std::nullopt,
        _network_server,
        _players, 
        std::nullopt

    };

    this->init();
    while (1) {
        handleNetworkMessages();
        context.dt = clock.restart().asSeconds();
        if (_players.size() < 2)
            continue;
        if (_function)
            _function(_ecs);
        _ecs.update(context);
    }
    return 0;
}

void ServerGameEngine::handleNetworkMessages() {
    /**
        Connection player -> id
    */
    _network_server.Update();
    coming_message c_msg = _network_server.ReadIncomingMessage();

    if (c_msg.id != GameEvents::NONE) {
        execCorrespondingFunction(c_msg.id, c_msg);
    }
}

void ServerGameEngine::execCorrespondingFunction(GameEvents event, coming_message c_msg) {
    switch (event) {
        case (GameEvents::CONNECTION_PLAYER):
            uint32_t id;
            c_msg.msg >> id;
            _players.push_back(id);
            std::cout << "Connect player " << id << std::endl;
            break;

        case (GameEvents::C_GAME_START):
            std::cout << "START GAME" << std::endl;
            _has_game_start = true;
            break;

        case (GameEvents::C_INPUT):
            std::cout << "Input to implement" << std::endl;
            break;

        default:
            std::cout << "EVENT " << uint32_t(event) << " IS NOT IMPLEMENTED" << std::endl;
            break;
    }
}
