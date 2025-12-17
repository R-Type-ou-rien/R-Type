#include "ServerGameEngine.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <ostream>
#include "Components/NetworkComponents.hpp"
#include "InputSystem.hpp"
#include "Network/Network.hpp"

ServerGameEngine::ServerGameEngine() : _network_server(4040, 100) {}

ServerGameEngine::~ServerGameEngine() {}

int ServerGameEngine::init() {
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<ComponentSenderSystem>();
    _ecs.systems.addSystem<PhysicsSystem>();
    _ecs.systems.addSystem<InputSystem>(_input_manager);

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

void ServerGameEngine::setOnPlayerConnect(std::function<void(std::uint32_t)> callback) {
    _onPlayerConnect = callback;
}

ECS& ServerGameEngine::getECS() {
    return _ecs;
}

int ServerGameEngine::run() {
    sf::Clock clock;
    system_context context = {
        0,        _ecs._textureManager, std::nullopt, _input_manager, std::nullopt, _network_server,
        _players, std::nullopt

    };

    this->init();
    const double target_dt = 1.0 / 60.0;
    while (1) {
        auto start_time = std::chrono::steady_clock::now();

        handleNetworkMessages();
        context.dt = clock.restart().asSeconds();
        if (_players.size() < 1)
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

    // Process all pending messages
    size_t processed_count = 0;
    const size_t max_messages_per_frame = 1000;  // Safety limit

    while (processed_count < max_messages_per_frame) {
        coming_message c_msg = _network_server.ReadIncomingMessage();

        if (c_msg.id == GameEvents::NONE) {
            break;
        }

        execCorrespondingFunction(c_msg.id, c_msg);
        processed_count++;
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
            if (_onPlayerConnect) {
                _onPlayerConnect(id);
            }
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
            InputPacket packet;
            c_msg.msg >> packet;
            std::cout << "Received Input: " << packet.action_name << " from " << c_msg.clientID << std::endl;

            // _input_manager.setReceivedAction(packet); // CAUSE OF BUG: Global input affects all players

            auto& registry = _ecs.registry;
            auto identities = registry.getEntities<NetworkIdentity>();

            for (auto entity : identities) {
                const auto& netId = registry.getComponentConst<NetworkIdentity>(entity);
                if (netId.owner_user_id == c_msg.clientID) {
                    if (registry.hasComponent<ActionScript>(entity)) {
                        auto& script = registry.getComponent<ActionScript>(entity);

                        // Construct a temporary context for the callback
                        system_context context = {
                            0,        _ecs._textureManager, std::nullopt, _input_manager, std::nullopt, _network_server,
                            _players, c_msg.clientID};

                        if (packet.state.justPressed && script.actionOnPressed.count(packet.action_name)) {
                            std::cout << "[Server] Executing ActionOnPressed: " << packet.action_name << " for entity "
                                      << entity << std::endl;
                            script.actionOnPressed[packet.action_name](registry, context, entity);
                        }
                        if (packet.state.justReleased && script.actionOnReleased.count(packet.action_name)) {
                            script.actionOnReleased[packet.action_name](registry, context, entity);
                        }
                        if (packet.state.pressed && script.actionPressed.count(packet.action_name)) {
                            std::cout << "[Server] Executing ActionPressed: " << packet.action_name << " for entity "
                                      << entity << std::endl;
                            script.actionPressed[packet.action_name](registry, context, entity);
                        } else if (packet.state.pressed) {
                            std::cout << "[Server] ActionPressed " << packet.action_name
                                      << " found but NOT in script.actionPressed map!" << std::endl;
                        }
                    } else {
                        std::cout << "[Server] Entity " << entity << " has no ActionScript!" << std::endl;
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
