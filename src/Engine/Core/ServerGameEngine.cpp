#ifdef CLIENT_BUILD
#undef CLIENT_BUILD
#endif
#ifndef SERVER_BUILD
#define SERVER_BUILD
#endif

#include "ServerGameEngine.hpp"
#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>
#include "Components/NetworkComponents.hpp"
#include "Context.hpp"
#include "GameEngineBase.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/serialize/StandardComponents_serialize.hpp"
#include "Components/serialize/score_component_serialize.hpp"
#include "../../RType/Common/Systems/health.hpp"
#include "../../RType/Common/Systems/score.hpp"
#include "../../RType/Common/Components/spawn.hpp"
#include "../../RType/Common/Components/shooter_component.hpp"
#include "../../RType/Common/Components/charged_shot.hpp"
#include "../../RType/Common/Components/team_component.hpp"
#include "../../RType/Common/Components/damage_component.hpp"
#include "../../RType/Common/Components/game_timer.hpp"
#include "../../RType/Common/Components/pod_component.hpp"
#include "../../RType/Common/Components/charged_shot.hpp"
#include "../../RType/Common/Components/scripted_spawn.hpp"
#include "../../RType/Common/Systems/behavior.hpp"
#include "../../RType/Common/Entities/Player/Player.hpp"
#include "../../RType/Common/Systems/spawn.hpp"
#include "CollisionSystem.hpp"
#include "../Lib/Systems/PhysicsSystem.hpp"
ServerGameEngine::ServerGameEngine()
    : _env(std::make_shared<Environment>(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::SERVER)) {
    _network = std::make_shared<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::SERVER);
    // No default lobby - wait for client requests (CREATE_LOBBY / JOIN_LOBBY)
}

int ServerGameEngine::init() {
    // Systems are added in GameManager::initSystems()
    // Only add server-specific systems here
    _ecs.systems.addSystem<ComponentSenderSystem>();

    registerNetworkComponent<sprite2D_component_s>();
    registerNetworkComponent<transform_component_s>();
    registerNetworkComponent<Velocity2D>();
    registerNetworkComponent<BoxCollisionComponent>();
    registerNetworkComponent<TagComponent>();
    registerNetworkComponent<HealthComponent>();
    registerNetworkComponent<EnemySpawnComponent>();
    registerNetworkComponent<ShooterComponent>();
    registerNetworkComponent<ChargedShotComponent>();
    registerNetworkComponent<TextComponent>();
    registerNetworkComponent<ResourceComponent>();
    registerNetworkComponent<BackgroundComponent>();
    registerNetworkComponent<PatternComponent>();
    registerNetworkComponent<ProjectileComponent>();
    registerNetworkComponent<TeamComponent>();
    registerNetworkComponent<DamageOnCollision>();
    registerNetworkComponent<NetworkIdentity>();
    registerNetworkComponent<::GameTimerComponent>();
    // AudioSourceComponent removed - audio is client-local and has std::string fields

    // R-Type specific components
    registerNetworkComponent<PodComponent>();
    registerNetworkComponent<PlayerPodComponent>();
    registerNetworkComponent<BehaviorComponent>();
    registerNetworkComponent<BossComponent>();
    registerNetworkComponent<BossSubEntityComponent>();
    registerNetworkComponent<ScoreComponent>();

    // Expose Server Engine services to Game Logic
    _env->addFunction("registerPlayer", std::function<void(uint32_t, std::shared_ptr<Player>)>(
                                            [this](uint32_t clientId, std::shared_ptr<Player> player) {
                                                if (!player)
                                                    return;
                                                _players[clientId] = player;
                                                _clientToEntityMap[clientId] = player->getId();
                                                _pendingFullState.insert(clientId);
                                                std::cout << "SERVER: Registered player entity " << player->getId()
                                                          << " for client " << clientId << std::endl;
                                            }));

    // Expose Lobby iteration for Game Manager
    _env->addFunction("forEachLobby",
                      std::function<void(std::function<void(uint32_t, int, const std::vector<uint32_t>&)>)>(
                          [this](std::function<void(uint32_t, int, const std::vector<uint32_t>&)> callback) {
                              for (const auto& [id, lobby] : _lobbyManager.getAllLobbies()) {
                                  std::vector<uint32_t> clientIds;
                                  for (const auto& client : lobby.getClients()) {
                                      clientIds.push_back(client.id);
                                  }
                                  callback(id, (int)lobby.getState(), clientIds);
                              }
                          }));

    return SUCCESS;
}

void ServerGameEngine::processNetworkEvents() {
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents();

    // Track which clients confirmed UDP this frame (may arrive before C_CONNECTION is processed)
    std::set<uint32_t> udpConfirmedThisFrame;

    // Collect UDP confirmations first
    if (pending.count(network::GameEvents::C_CONFIRM_UDP)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CONFIRM_UDP)) {
            udpConfirmedThisFrame.insert(msg.header.user_id);
        }
    }

    // Handle new connections - only register client, do not auto-join or spawn
    if (pending.count(network::GameEvents::C_CONNECTION)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CONNECTION)) {
            uint32_t newClientId = msg.header.user_id;
            _lobbyManager.onClientConnected(newClientId);
            std::cout << "SERVER: Client " << newClientId << " connected. Waiting for lobby commands." << std::endl;
        }
    }

    // Handle lobby creation/join - update lobby manager
    if (pending.count(network::GameEvents::S_ROOM_JOINED)) {
        for (auto msg : pending.at(network::GameEvents::S_ROOM_JOINED)) {
            network::lobby_in_info info;
            msg >> info;
            uint32_t clientId = msg.header.user_id;

            // First ensure client is registered
            _lobbyManager.onClientConnected(clientId, "Player" + std::to_string(clientId));

            // Check if lobby exists, if not create it
            auto lobbyOpt = _lobbyManager.getLobby(info.id);
            if (!lobbyOpt) {
                // Create lobby using public API - the manager will give it a new ID,
                // but we'll use joinLobby which updates client mapping
                auto& newLobby = _lobbyManager.createLobby(info.name, 4);
                newLobby.setHostId(info.hostId);
                // Join client to the NEW lobby (using its assigned ID)
                _lobbyManager.joinLobby(newLobby.getId(), clientId);
                std::cout << "SERVER_ENGINE: Created lobby " << newLobby.getId() << " (" << info.name << "), client "
                          << clientId << " joined" << std::endl;
            } else {
                // Join existing lobby
                _lobbyManager.joinLobby(info.id, clientId);
                std::cout << "SERVER_ENGINE: Client " << clientId << " joined existing lobby " << info.id << std::endl;
            }
        }
    }

    // Handle C_READY
    if (pending.count(network::GameEvents::C_READY)) {
        for (const auto& msg : pending.at(network::GameEvents::C_READY)) {
            uint32_t clientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(clientId);
            if (lobbyOpt) {
                lobbyOpt->get().setPlayerReady(clientId, true);
                std::cout << "SERVER: Client " << clientId << " is ready" << std::endl;

                auto network_instance = _network->getNetworkInstance();
                if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                    auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                    for (const auto& client : lobbyOpt->get().getClients()) {
                        network::message<network::GameEvents> reply;
                        reply.header.id = network::GameEvents::S_READY_RETURN;
                        reply << clientId;
                        server->AddMessageToPlayer(network::GameEvents::S_READY_RETURN, client.id, reply);
                    }
                }
            }
        }
    }

    // Handle C_CANCEL_READY
    if (pending.count(network::GameEvents::C_CANCEL_READY)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CANCEL_READY)) {
            uint32_t clientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(clientId);
            if (lobbyOpt) {
                lobbyOpt->get().setPlayerReady(clientId, false);
                std::cout << "SERVER: Client " << clientId << " cancelled ready" << std::endl;

                auto network_instance = _network->getNetworkInstance();
                if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                    auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                    for (const auto& client : lobbyOpt->get().getClients()) {
                        network::message<network::GameEvents> reply;
                        reply.header.id = network::GameEvents::S_CANCEL_READY_BROADCAST;
                        reply << clientId;
                        server->AddMessageToPlayer(network::GameEvents::S_CANCEL_READY_BROADCAST, client.id, reply);
                    }
                }
            }
        }
    }

    // Handle game start - spawn players when host starts the game
    if (pending.count(network::GameEvents::S_GAME_START)) {
        for (const auto& msg : pending.at(network::GameEvents::S_GAME_START)) {
            // The clientId here is the host who started the game
            uint32_t hostClientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(hostClientId);
            if (!lobbyOpt) {
                std::cout << "SERVER: S_GAME_START received but host " << hostClientId << " not in any lobby"
                          << std::endl;
                continue;
            }

            auto& lobby = lobbyOpt->get();
            // Verify host
            if (!lobby.isHost(hostClientId)) {
                std::cout << "SERVER: Client " << hostClientId << " tried to start game but is not host" << std::endl;
                continue;
            }

            lobby.setState(engine::core::Lobby::State::IN_GAME);
            std::cout << "SERVER: Game starting in lobby " << lobby.getId() << " (" << lobby.getName() << ")"
                      << std::endl;

            // Broadcast S_GAME_START
            auto network_instance = _network->getNetworkInstance();
            if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                for (const auto& client : lobby.getClients()) {
                    network::message<network::GameEvents> reply;
                    reply.header.id = network::GameEvents::S_GAME_START;
                    // Payload? Client implementation (Step 709) doesn't read payload for S_GAME_START.
                    // Just sends header.
                    server->AddMessageToPlayer(network::GameEvents::S_GAME_START, client.id, reply);
                }
            }
        }
    }

    // Handle player leaving lobby (before full disconnect)
    if (pending.count(network::GameEvents::S_PLAYER_LEAVE)) {
        for (const auto& msg : pending.at(network::GameEvents::S_PLAYER_LEAVE)) {
            uint32_t clientId = msg.header.user_id;
            _lobbyManager.leaveLobby(clientId);
            std::cout << "SERVER_ENGINE: Client " << clientId << " left lobby" << std::endl;
        }
    }

    // Handle disconnections
    if (pending.count(network::GameEvents::C_DISCONNECT)) {
        for (const auto& msg : pending.at(network::GameEvents::C_DISCONNECT)) {
            uint32_t clientId = msg.header.user_id;
            _lobbyManager.onClientDisconnected(clientId);
            _clientToEntityMap.erase(clientId);  // Clean up entity mapping
            _players.erase(clientId);  // Remove player ownership, triggering destructor and entity destruction
            std::cout << "SERVER: Client " << clientId << " disconnected." << std::endl;
        }
    }

    // Handle inputs
    if (pending.count(network::GameEvents::C_INPUT)) {
        auto& input_messages = pending.at(network::GameEvents::C_INPUT);
        for (auto& msg : input_messages) {
            ActionPacket packet;
            msg >> packet;
            updateActions(packet, msg.header.user_id);
        }
    }

    // NOW send full game state to clients whose UDP has been confirmed
    // This is done AFTER C_CONNECTION so that new players are created first
    for (uint32_t clientId : udpConfirmedThisFrame) {
        // Check if client is in pending list (was added by C_CONNECTION handler)
        if (_pendingFullState.find(clientId) == _pendingFullState.end()) {
            std::cout << "SERVER: UDP confirmed for client " << clientId << " but not in pending list yet" << std::endl;
            continue;
        }
        _pendingFullState.erase(clientId);

        std::cout << "SERVER: UDP confirmed for client " << clientId << ", sending full game state" << std::endl;

        auto network_instance = _network->getNetworkInstance();
        if (!std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
            continue;
        }
        auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

        SerializationContext s_ctx = {_texture_manager};
        auto& pools = _ecs.registry.getComponentPools();

        // Send full game state to this client
        int totalPacketsSent = 0;
        for (auto& [type, pool] : pools) {
            uint32_t typeHash = pool->getTypeHash();
            if (_networkedComponentTypes.find(typeHash) == _networkedComponentTypes.end()) {
                continue;
            }

            auto& entities = pool->getIdList();
            for (auto entity : entities) {
                if (!_ecs.registry.hasComponent<NetworkIdentity>(entity)) {
                    continue;
                }

                ComponentPacket packet = pool->createPacket(entity, s_ctx);
                packet.entity_guid = _ecs.registry.getConstComponent<NetworkIdentity>(entity).guid;
                server->AddMessageToPlayer(network::GameEvents::S_SNAPSHOT, clientId, packet);
                totalPacketsSent++;

                // Log sprite components being sent
                if (typeHash == 134294793) {
                    std::cout << "SERVER: [POST-UDP] Sending SPRITE for entity guid=" << packet.entity_guid
                              << " to client " << clientId << std::endl;
                }
            }
        }
        std::cout << "SERVER: [POST-UDP] Sent " << totalPacketsSent << " component packets to client " << clientId
                  << std::endl;

        // Tell the client which entity is their player
        auto playerIt = _players.find(clientId);
        if (playerIt != _players.end()) {
            network::AssignPlayerEntityPacket assignPacket;
            assignPacket.entityId = playerIt->second->getId();
            server->AddMessageToPlayer(network::GameEvents::S_ASSIGN_PLAYER_ENTITY, clientId, assignPacket);
        }
    }
}

void ServerGameEngine::updateActions(ActionPacket& packet, uint32_t clientId) {
    auto it = _clientToEntityMap.find(clientId);

    input_manager.updateActionFromPacket(packet, clientId);
}

int ServerGameEngine::run() {
    system_context ctx = {
        0,         _currentTick, _texture_manager, _sound_manager,           _music_manager, input_manager,
        *_network, {},           &_lobbyManager,   &_networkedComponentTypes};
    auto last_time = std::chrono::high_resolution_clock::now();

    init();

    if (_init_function) {
        std::cout << "SERVER: Running init function" << std::endl;
        _init_function(_env, input_manager);
    }

    while (1) {
        auto now = std::chrono::high_resolution_clock::now();
        ctx.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        processNetworkEvents();

        if (_loop_function) {
            _loop_function(_env, input_manager);
        }

        // Populate active clients for systems
        ctx.active_clients.clear();
        for (const auto& [lobbyId, lobby] : _lobbyManager.getAllLobbies()) {
            if (lobby.getState() == engine::core::Lobby::State::IN_GAME) {
                for (const auto& client : lobby.getClients()) {
                    ctx.active_clients.push_back(client.id);
                }
            }
        }

        _ecs.update(ctx);

        // Reset one-frame input flags (justPressed, justReleased) after processing
        input_manager.resetFrameFlags();

        _currentTick++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return SUCCESS;
}