#include "ServerGameEngine.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <ostream>
#include "Network/Network.hpp"

ServerGameEngine::ServerGameEngine() : _network_server(4040, 100) {}

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
    system_context context = {0,        _ecs._textureManager, std::nullopt, std::nullopt, std::nullopt, _network_server,
                              _players, std::nullopt

    };

    this->init();
    const double target_dt = 1.0 / 60.0;
    while (1) {
        auto start_time = std::chrono::steady_clock::now();

        handleNetworkMessages();
        context.dt = clock.restart().asSeconds();
        if (_players.size() < 2)
            continue;
        if (_function)
            _function(_ecs);
        _ecs.update(context);

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        if (elapsed.count() < target_dt) {
            std::this_thread::sleep_for(std::chrono::duration<double>(target_dt - elapsed.count()));
        }
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
            _network_server.AddMessageToPlayer(GameEvents::S_SEND_ID, id, id);
            break;

        case (GameEvents::C_GAME_START): {
            std::cout << "START GAME" << std::endl;
            _has_game_start = true;
            auto& pools = _ecs.registry.getPools();
            for (auto& [type, pool] : pools) {
                pool->markAllUpdated();
            }
            std::cout << "Resynchronizing all components for Game Start" << std::endl;
            break;
        }

        case (GameEvents::C_INPUT): {
            std::string action;
            bool pressed;
            c_msg.msg >> pressed >> action;
            // std::cout << "Received Input: " << action << " " << pressed << " from " << c_msg.clientID << std::endl;

            auto& net_ids = _ecs.registry.getView<NetworkIdentity>();
            auto& ids = _ecs.registry.getEntities<NetworkIdentity>();

            for (size_t i = 0; i < net_ids.size(); ++i) {
                if (net_ids[i].owner_user_id == c_msg.clientID) {
                    Entity e = ids[i];
                    if (_ecs.registry.hasComponent<Velocity2D>(e)) {
                        auto& vel = _ecs.registry.getComponent<Velocity2D>(e);
                        if (action == "move_left")
                            vel.vx = pressed ? -100.0f : 0.0f;
                        if (action == "move_right")
                            vel.vx = pressed ? 100.0f : 0.0f;
                        if (action == "move_up")
                            vel.vy = pressed ? -100.0f : 0.0f;
                        if (action == "move_down")
                            vel.vy = pressed ? 100.0f : 0.0f;
                    }
                    break;
                }
            }
            break;
        }

        default:
            // std::cout << "EVENT " << uint32_t(event) << " IS NOT IMPLEMENTED" << std::endl;
            break;
    }
}
