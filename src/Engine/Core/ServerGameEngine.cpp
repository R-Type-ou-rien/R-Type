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
#include <set>
#include <memory>
#include <utility>
#include <thread>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <string>
#include <algorithm>
#include "Components/NetworkComponents.hpp"
#include "Components/LobbyIdComponent.hpp"
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
#include "../../RType/Common/Components/scripted_spawn.hpp"
#include "../Lib/Components/LobbyIdComponent.hpp"
#include "../Lib/Utils/LobbyUtils.hpp"
#include "../../RType/Common/Systems/behavior.hpp"
#include "../../RType/Common/Entities/Player/Player.hpp"
#include "Components/Sprite/Sprite2D.hpp"
#include "Components/Sprite/AnimatedSprite2D.hpp"

#include "../../RType/Common/Systems/spawn.hpp"
#include "CollisionSystem.hpp"
#include "../Lib/Systems/PhysicsSystem.hpp"

ServerGameEngine::ServerGameEngine(std::string ip)
    : _env(std::make_shared<Environment>(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::SERVER)) {
    _network = std::make_shared<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::SERVER);
}

int ServerGameEngine::init() {
    _ecs.systems.addSystem<ComponentSenderSystem>();

    registerNetworkComponent<Sprite2D>();
    registerNetworkComponent<AnimatedSprite2D>();
    registerNetworkComponent<sprite2D_component_s>();
    registerNetworkComponent<transform_component_s>();
    registerNetworkComponent<Velocity2D>();
    registerNetworkComponent<BoxCollisionComponent>();
    registerNetworkComponent<TagComponent>();
    registerNetworkComponent<TextComponent>();
    registerNetworkComponent<ResourceComponent>();
    registerNetworkComponent<BackgroundComponent>();
    registerNetworkComponent<PatternComponent>();
    registerNetworkComponent<NetworkIdentity>();
    registerNetworkComponent<::GameTimerComponent>();

    registerNetworkComponent<PodComponent>();
    registerNetworkComponent<PlayerPodComponent>();
    registerNetworkComponent<BehaviorComponent>();
    registerNetworkComponent<BossComponent>();
    registerNetworkComponent<BossSubEntityComponent>();
    registerNetworkComponent<ScoreComponent>();
    registerNetworkComponent<AudioSourceComponent>();

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

    _env->addFunction("forEachLobby",
                      std::function<void(std::function<void(uint32_t, int, const std::vector<uint32_t>&)>)>(
                          [this](std::function<void(uint32_t, int, const std::vector<uint32_t>&)> callback) {
                              for (const auto& [id, lobby] : _lobbyManager.getAllLobbies()) {
                                  std::vector<uint32_t> clientIds;
                                  for (const auto& client : lobby.getClients()) {
                                      clientIds.push_back(client.id);
                                  }
                                  callback(id, static_cast<int>(lobby.getState()), clientIds);
                              }
                          }));

    _env->addFunction(
        "broadcastGameOver",
        std::function<void(uint32_t, bool, const std::vector<std::tuple<uint32_t, int, bool>>&)>(
            [this](uint32_t lobbyId, bool victory, const std::vector<std::tuple<uint32_t, int, bool>>& scores) {
                network::GameOverPacket packet;
                packet.victory = victory;
                packet.player_count = std::min(static_cast<size_t>(8), scores.size());
                for (size_t i = 0; i < packet.player_count; ++i) {
                    packet.players[i].client_id = std::get<0>(scores[i]);
                    packet.players[i].score = std::get<1>(scores[i]);
                    packet.players[i].is_alive = std::get<2>(scores[i]);
                }

                auto network_instance = _network->getNetworkInstance();
                if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                    auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

                    auto lobbyOpt = _lobbyManager.getLobby(lobbyId);
                    if (lobbyOpt) {
                        // Broadcast S_GAME_OVER
                        for (const auto& client : lobbyOpt->get().getClients()) {
                            network::message<network::GameEvents> msg;
                            msg.header.id = network::GameEvents::S_GAME_OVER;
                            msg << packet;
                            server->AddMessageToPlayer(network::GameEvents::S_GAME_OVER, client.id, msg);
                        }

                        if (_env->hasFunction("getECS")) {
                            auto& ecs = _env->getECS();
                            std::unordered_set<Entity> entitiesToDestroy;

                            auto& lobbyIds = ecs.registry.getEntities<LobbyIdComponent>();
                            for (auto entity : lobbyIds) {
                                if (ecs.registry.hasComponent<LobbyIdComponent>(entity)) {
                                    auto& lobbyComp = ecs.registry.getComponent<LobbyIdComponent>(entity);
                                    if (lobbyComp.lobby_id == lobbyId) {
                                        entitiesToDestroy.insert(entity);
                                    }
                                }
                            }

                            auto& taggedEntities = ecs.registry.getEntities<TagComponent>();
                            for (auto entity : taggedEntities) {
                                if (!ecs.registry.hasComponent<TagComponent>(entity))
                                    continue;
                                auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);

                                bool isDynamicInfo = false;
                                for (const auto& tag : tags.tags) {
                                    if (tag == "PROJECTILE" || tag == "ENEMY_PROJECTILE" ||
                                        tag == "FRIENDLY_PROJECTILE" || tag == "ENEMY" || tag == "BOSS" ||
                                        tag == "OBSTACLE" || tag == "POD" || tag == "POWERUP" ||
                                        tag == "LEADERBOARD_DATA" || tag == "PLAYER") {
                                        isDynamicInfo = true;
                                        break;
                                    }
                                }

                                if (isDynamicInfo) {
                                    uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                                    if (entLobby == lobbyId || entLobby == 0) {
                                        entitiesToDestroy.insert(entity);
                                    }
                                }
                            }

                            for (auto entity : entitiesToDestroy) {
                                ecs.registry.destroyEntity(entity);
                            }
                            std::cout << "SERVER: Destroyed " << entitiesToDestroy.size() << " entities for lobby "
                                      << lobbyId << std::endl;
                        }

                        lobbyOpt->get().setState(engine::core::Lobby::State::WAITING);

                        for (const auto& client : lobbyOpt->get().getClients()) {
                            lobbyOpt->get().setPlayerReady(client.id, false);
                        }

                        for (const auto& client : lobbyOpt->get().getClients()) {
                            for (const auto& receiver : lobbyOpt->get().getClients()) {
                                network::message<network::GameEvents> reply;
                                reply.header.id = network::GameEvents::S_CANCEL_READY_BROADCAST;
                                reply << client.id;
                                server->AddMessageToPlayer(network::GameEvents::S_CANCEL_READY_BROADCAST, receiver.id,
                                                           reply);
                            }
                        }

                        server->AddMessageToLobby(network::GameEvents::S_RETURN_TO_LOBBY, lobbyId, 0);

                        std::cout << "SERVER: Broadcasted Game Over for lobby " << lobbyId << std::endl;
                    }
                }
            }));

    return SUCCESS;
}

void ServerGameEngine::processNetworkEvents() {
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents();

    std::set<uint32_t> udpConfirmedThisFrame;

    if (pending.count(network::GameEvents::C_CONFIRM_UDP)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CONFIRM_UDP)) {
            udpConfirmedThisFrame.insert(msg.header.user_id);
        }
    }

    if (pending.count(network::GameEvents::C_CONNECTION)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CONNECTION)) {
            uint32_t newClientId = msg.header.user_id;
            _lobbyManager.onClientConnected(newClientId);
            std::cout << "SERVER: Client " << newClientId << " connected. Waiting for lobby commands." << std::endl;
        }
    }
    if (pending.count(network::GameEvents::S_ROOM_JOINED)) {
        for (auto msg : pending.at(network::GameEvents::S_ROOM_JOINED)) {
            network::lobby_in_info info;
            msg >> info;
            uint32_t clientId = msg.header.user_id;

            _lobbyManager.onClientConnected(clientId, "Player" + std::to_string(clientId));

            auto lobbyOpt = _lobbyManager.getLobby(info.id);
            if (!lobbyOpt) {
                auto& newLobby = _lobbyManager.createLobby(info.name, 4);
                newLobby.setHostId(info.hostId);
                _lobbyManager.joinLobby(newLobby.getId(), clientId);
                std::cout << "SERVER_ENGINE: Created lobby " << newLobby.getId() << " (" << info.name << "), client "
                          << clientId << " joined" << std::endl;
            } else {
                _lobbyManager.joinLobby(info.id, clientId);
                std::cout << "SERVER_ENGINE: Client " << clientId << " joined existing lobby " << info.id << std::endl;
            }
        }
    }

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
            } else {
                std::cout << "SERVER: Client " << clientId << " sent C_READY but is not in any lobby!" << std::endl;
            }
        }
    }

    // Handle C_CANCEL_READY
    if (pending.count(network::GameEvents::C_CANCEL_READY)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CANCEL_READY)) {
            uint32_t clientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(clientId);
            if (lobbyOpt) {
                auto& lobby = lobbyOpt->get();
                lobby.setPlayerReady(clientId, false);
                std::cout << "SERVER: Client " << clientId << " cancelled ready" << std::endl;

                auto network_instance = _network->getNetworkInstance();
                if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                    auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

                    if (lobby.getState() == engine::core::Lobby::State::IN_GAME) {
                        uint32_t lobbyId = lobby.getId();
                        std::cout << "SERVER: Client " << clientId << " Aborted Game (C_CANCEL_READY). Resetting Lobby "
                                  << lobbyId << std::endl;
                        lobby.setState(engine::core::Lobby::State::WAITING);

                        auto& ecs = _env->getECS();
                        std::unordered_set<Entity> entitiesToDestroy;

                        auto& lobbyIds = ecs.registry.getEntities<LobbyIdComponent>();
                        for (auto entity : lobbyIds) {
                            if (ecs.registry.hasComponent<LobbyIdComponent>(entity)) {
                                if (ecs.registry.getComponent<LobbyIdComponent>(entity).lobby_id == lobbyId) {
                                    entitiesToDestroy.insert(entity);
                                }
                            }
                        }

                        auto& taggedEntities = ecs.registry.getEntities<TagComponent>();
                        for (auto entity : taggedEntities) {
                            if (!ecs.registry.hasComponent<TagComponent>(entity))
                                continue;
                            auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
                            bool isDynamic = false;
                            for (const auto& tag : tags.tags) {
                                if (tag == "PROJECTILE" || tag == "ENEMY_PROJECTILE" || tag == "FRIENDLY_PROJECTILE" ||
                                    tag == "ENEMY" || tag == "BOSS" || tag == "OBSTACLE" || tag == "POD" ||
                                    tag == "POWERUP" || tag == "LEADERBOARD_DATA" || tag == "PLAYER" || tag == "AI") {
                                    isDynamic = true;
                                    break;
                                }
                            }
                            if (isDynamic) {
                                uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                                if (entLobby == lobbyId || entLobby == 0) {
                                    entitiesToDestroy.insert(entity);
                                }
                            }
                        }

                        auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
                        for (auto entity : spawners) {
                            if (ecs.registry.hasComponent<EnemySpawnComponent>(entity)) {
                                const auto& spawnComp = ecs.registry.getConstComponent<EnemySpawnComponent>(entity);
                                if (spawnComp.lobby_id == lobbyId) {
                                    entitiesToDestroy.insert(entity);
                                }
                            }
                        }

                        auto& timers = ecs.registry.getEntities<GameTimerComponent>();
                        for (auto entity : timers) {
                            uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                            if (entLobby == lobbyId) {
                                entitiesToDestroy.insert(entity);
                            }
                        }

                        // Cleanup Scores
                        auto& scores = ecs.registry.getEntities<ScoreComponent>();
                        for (auto entity : scores) {
                            uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                            if (entLobby == lobbyId) {
                                entitiesToDestroy.insert(entity);
                            }
                        }

                        for (auto entity : entitiesToDestroy) {
                            ecs.registry.destroyEntity(entity);
                        }
                        std::cout << "SERVER: Destroyed " << entitiesToDestroy.size() << " entities for lobby "
                                  << lobbyId << " (Abort Reset)" << std::endl;

                        // Reset all players ready
                        for (const auto& client : lobby.getClients()) {
                            lobby.setPlayerReady(client.id, false);
                        }

                        // Broadcast S_RETURN_TO_LOBBY to force clients back
                        server->AddMessageToLobby(network::GameEvents::S_RETURN_TO_LOBBY, lobbyId, 0);
                    }

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

    if (pending.count(network::GameEvents::S_GAME_START)) {
        for (const auto& msg : pending.at(network::GameEvents::S_GAME_START)) {
            uint32_t hostClientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(hostClientId);
            if (!lobbyOpt) {
                std::cout << "SERVER: S_GAME_START received but host " << hostClientId << " not in any lobby"
                          << std::endl;
                continue;
            }

            auto& lobby = lobbyOpt->get();
            if (!lobby.isHost(hostClientId)) {
                std::cout << "SERVER: Client " << hostClientId << " tried to start game but is not host" << std::endl;
                continue;
            }

            lobby.setState(engine::core::Lobby::State::IN_GAME);
            std::cout << "SERVER: Game starting in lobby " << lobby.getId() << " (" << lobby.getName() << ")"
                      << std::endl;

            auto network_instance = _network->getNetworkInstance();
            if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                for (const auto& client : lobby.getClients()) {
                    network::message<network::GameEvents> reply;
                    reply.header.id = network::GameEvents::S_GAME_START;
                    server->AddMessageToPlayer(network::GameEvents::S_GAME_START, client.id, reply);
                }
            }
        }
    }

    if (pending.count(network::GameEvents::S_PLAYER_LEAVE)) {
        for (const auto& msg : pending.at(network::GameEvents::S_PLAYER_LEAVE)) {
            uint32_t clientId = msg.header.user_id;
            _lobbyManager.leaveLobby(clientId);
            std::cout << "SERVER_ENGINE: Client " << clientId << " left lobby" << std::endl;
        }
    }

    if (pending.count(network::GameEvents::C_DISCONNECT)) {
        for (const auto& msg : pending.at(network::GameEvents::C_DISCONNECT)) {
            uint32_t clientId = msg.header.user_id;
            _lobbyManager.onClientDisconnected(clientId);
            _clientToEntityMap.erase(clientId);
            _players.erase(clientId);
            std::cout << "SERVER: Client " << clientId << " disconnected." << std::endl;
        }
    }

    if (pending.count(network::GameEvents::C_INPUT)) {
        auto& input_messages = pending.at(network::GameEvents::C_INPUT);
        for (auto& msg : input_messages) {
            ActionPacket packet;
            msg >> packet;
            updateActions(packet, msg.header.user_id);
        }
    }

    if (pending.count(network::GameEvents::C_TEAM_CHAT)) {
        auto& msgs = pending.at(network::GameEvents::C_TEAM_CHAT);
        for (auto& msg : msgs) {
            uint32_t clientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(clientId);
            if (lobbyOpt) {
                network::chat_message chatMsg;
                if (msg.body.size() >= 256) {
                    char rawMsg[256];
                    std::memcpy(rawMsg, msg.body.data(), 256);
                    network::chat_message broadcastMsg;
                    broadcastMsg.sender_id = clientId;
                    std::string senderName = "Player " + std::to_string(clientId);
                    for (const auto& client : lobbyOpt->get().getClients()) {
                        if (client.id == clientId) {
                            senderName = client.name;
                            break;
                        }
                    }
                    std::strncpy(broadcastMsg.sender_name, senderName.c_str(), 31);
                    std::strncpy(broadcastMsg.message, rawMsg, 255);

                    std::cout << "SERVER: Chat from " << senderName << ": " << broadcastMsg.message << std::endl;

                    // Broadcast to lobby
                    auto network_instance = _network->getNetworkInstance();
                    if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                        auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                        for (const auto& client : lobbyOpt->get().getClients()) {
                            network::message<network::GameEvents> reply;
                            reply.header.id = network::GameEvents::S_TEAM_CHAT;
                            reply << broadcastMsg;
                            server->AddMessageToPlayer(network::GameEvents::S_TEAM_CHAT, client.id, reply);
                        }
                    }
                }
            } else {
                std::cout << "SERVER: Client " << clientId << " sent C_TEAM_CHAT but is not in any lobby!" << std::endl;
            }
        }
    }

    if (pending.count(network::GameEvents::S_RETURN_TO_LOBBY)) {
        for (const auto& msg : pending.at(network::GameEvents::S_RETURN_TO_LOBBY)) {
            uint32_t clientId = msg.header.user_id;
            auto lobbyOpt = _lobbyManager.getLobbyForClient(clientId);
            if (lobbyOpt) {
                auto& lobby = lobbyOpt->get();
                uint32_t lobbyId = lobby.getId();
                std::cout << "SERVER: Client " << clientId << " requested Return to Lobby. Resetting Lobby " << lobbyId
                          << std::endl;

                lobby.setState(engine::core::Lobby::State::WAITING);

                auto& ecs = _env->getECS();
                std::unordered_set<Entity> entitiesToDestroy;

                auto& lobbyIds = ecs.registry.getEntities<LobbyIdComponent>();
                for (auto entity : lobbyIds) {
                    if (ecs.registry.hasComponent<LobbyIdComponent>(entity)) {
                        if (ecs.registry.getComponent<LobbyIdComponent>(entity).lobby_id == lobbyId) {
                            entitiesToDestroy.insert(entity);
                        }
                    }
                }
                // Cleanup via Tags (Ghosts)
                auto& taggedEntities = ecs.registry.getEntities<TagComponent>();
                for (auto entity : taggedEntities) {
                    if (!ecs.registry.hasComponent<TagComponent>(entity))
                        continue;
                    auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
                    bool isDynamic = false;
                    for (const auto& tag : tags.tags) {
                        if (tag == "PROJECTILE" || tag == "ENEMY_PROJECTILE" || tag == "FRIENDLY_PROJECTILE" ||
                            tag == "ENEMY" || tag == "BOSS" || tag == "OBSTACLE" || tag == "POD" || tag == "POWERUP" ||
                            tag == "LEADERBOARD_DATA" || tag == "PLAYER" || tag == "AI") {
                            isDynamic = true;
                            break;
                        }
                    }
                    if (isDynamic) {
                        uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                        if (entLobby == lobbyId || entLobby == 0) {
                            entitiesToDestroy.insert(entity);
                        }
                    }
                }

                auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
                for (auto entity : spawners) {
                    if (ecs.registry.hasComponent<EnemySpawnComponent>(entity)) {
                        const auto& spawnComp = ecs.registry.getConstComponent<EnemySpawnComponent>(entity);
                        if (spawnComp.lobby_id == lobbyId) {
                            entitiesToDestroy.insert(entity);
                        }
                    }
                }

                auto& timers = ecs.registry.getEntities<GameTimerComponent>();
                for (auto entity : timers) {
                    uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                    if (entLobby == lobbyId) {
                        entitiesToDestroy.insert(entity);
                    }
                }

                auto& scores = ecs.registry.getEntities<ScoreComponent>();
                for (auto entity : scores) {
                    uint32_t entLobby = engine::utils::getLobbyId(ecs.registry, entity);
                    if (entLobby == lobbyId) {
                        entitiesToDestroy.insert(entity);
                    }
                }

                for (auto entity : entitiesToDestroy) {
                    ecs.registry.destroyEntity(entity);
                }
                std::cout << "SERVER: Destroyed " << entitiesToDestroy.size() << " entities for lobby " << lobbyId
                          << " (Manual Reset)" << std::endl;

                for (const auto& client : lobby.getClients()) {
                    lobby.setPlayerReady(client.id, false);
                }
                auto network_instance = _network->getNetworkInstance();
                if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                    auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                    server->AddMessageToLobby(network::GameEvents::S_RETURN_TO_LOBBY, lobbyId, 0);

                    for (const auto& client : lobby.getClients()) {
                        for (const auto& receiver : lobby.getClients()) {
                            network::message<network::GameEvents> reply;
                            reply.header.id = network::GameEvents::S_CANCEL_READY_BROADCAST;
                            reply << client.id;
                            server->AddMessageToPlayer(network::GameEvents::S_CANCEL_READY_BROADCAST, receiver.id,
                                                       reply);
                        }
                    }
                }
            }
        }
    }

    for (uint32_t clientId : udpConfirmedThisFrame) {
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
        uint32_t clientLobbyId = 0;
        auto lobbyOpt = _lobbyManager.getLobbyForClient(clientId);
        if (lobbyOpt.has_value()) {
            clientLobbyId = lobbyOpt->get().getId();
        }

        SerializationContext s_ctx = {_texture_manager};
        auto& pools = _ecs.registry.getComponentPools();

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

                uint32_t entityLobbyId = engine::utils::getLobbyId(_ecs.registry, entity);
                if (entityLobbyId != 0 && entityLobbyId != clientLobbyId) {
                    continue;
                }

                ComponentPacket packet = pool->createPacket(entity, s_ctx);
                packet.entity_guid = _ecs.registry.getConstComponent<NetworkIdentity>(entity).guid;
                packet.owner_id = _ecs.registry.getConstComponent<NetworkIdentity>(entity).ownerId;
                server->AddMessageToPlayer(network::GameEvents::S_SNAPSHOT, clientId, packet);
                totalPacketsSent++;
            }
        }

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

        ctx.active_clients.clear();
        for (const auto& [lobbyId, lobby] : _lobbyManager.getAllLobbies()) {
            if (lobby.getState() == engine::core::Lobby::State::IN_GAME) {
                for (const auto& client : lobby.getClients()) {
                    ctx.active_clients.push_back(client.id);
                }
            }
        }

        _ecs.update(ctx);

        input_manager.resetFrameFlags();

        _currentTick++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return SUCCESS;
}
