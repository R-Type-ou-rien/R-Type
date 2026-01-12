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
#include "../../RType/Common/Systems/health.hpp"
#include "../../RType/Common/Components/spawn.hpp"
#include "../../RType/Common/Components/shooter_component.hpp"
#include "../../RType/Common/Components/charged_shot.hpp"
#include "../../RType/Common/Components/team_component.hpp"
#include "../../RType/Common/Components/damage_component.hpp"
#include "../../RType/Common/Components/game_timer.hpp"
#include "../../RType/Common/Entities/Player/Player.hpp"

ServerGameEngine::ServerGameEngine() {
    _network = std::make_unique<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::SERVER);
    // Create a default lobby that players can join
    _lobbyManager.createLobby("Default Lobby", 4);
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
    registerNetworkComponent<AudioSourceComponent>();

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

    // Handle new connections
    if (pending.count(network::GameEvents::C_CONNECTION)) {
        for (const auto& msg : pending.at(network::GameEvents::C_CONNECTION)) {
            uint32_t newClientId = msg.header.user_id;
            _lobbyManager.onClientConnected(newClientId);

            // Get player count BEFORE joining to calculate correct spawn position
            auto lobbyBeforeJoin = _lobbyManager.getLobby(1);
            size_t playerCountBeforeJoin = lobbyBeforeJoin ? lobbyBeforeJoin->get().getPlayerCount() : 0;

            // Automatically join the default lobby for now. A real game would have lobby selection.
            if (_lobbyManager.joinLobby(1, newClientId)) {
                std::cout << "SERVER: Client " << newClientId << " joined lobby 1." << std::endl;
                auto lobby = _lobbyManager.getLobby(1);
                if (lobby) {
                    // Force state to IN_GAME for testing if it's waiting
                    if (lobby->get().getState() == engine::core::Lobby::State::WAITING) {
                        std::cout << "SERVER: Auto-starting lobby 1 state to IN_GAME" << std::endl;
                        lobby->get().setState(engine::core::Lobby::State::IN_GAME);
                    }

                    // Spawn player for the new client
                    float startX = 100.0f;
                    float startY = 100.0f + (playerCountBeforeJoin * 100.0f);

                    std::cout << "SERVER: Spawning player for client " << newClientId << " at " << startX << ", "
                              << startY << std::endl;

                    auto newPlayer = std::make_shared<Player>(_ecs, _texture_manager, std::make_pair(startX, startY));

                    // Set player texture so it's visible
                    newPlayer->setTexture("src/RType/Common/content/sprites/r-typesheet42.gif");
                    newPlayer->setTextureDimension({0, 0, 33, 17});  // Frame dimensions for player sprite

                    // Setup basic player stats
                    newPlayer->setLifePoint(100);
                    newPlayer->setTeam(TeamComponent::Team::ALLY);
                    newPlayer->addCollisionTag("AI");
                    newPlayer->addCollisionTag("ENEMY_PROJECTILE");
                    newPlayer->addCollisionTag("OBSTACLE");
                    newPlayer->addCollisionTag("ITEM");
                    newPlayer->addCollisionTag("WALL");

                    // Add NetworkIdentity so it gets replicated and accepts inputs
                    _ecs.registry.addComponent<NetworkIdentity>(newPlayer->getId(), {newPlayer->getId(), newClientId});

                    // Store player
                    _players[newClientId] = newPlayer;
                    _clientToEntityMap[newClientId] = newPlayer->getId();

                    // Add to pending list - full state will be sent after UDP confirmation
                    _pendingFullState.insert(newClientId);

                    Entity playerId = newPlayer->getId();
                    if (_ecs.registry.hasComponent<sprite2D_component_s>(playerId)) {
                        auto& playerSprite = _ecs.registry.getConstComponent<sprite2D_component_s>(playerId);
                        auto textureName = _texture_manager.get_name(playerSprite.handle);
                        std::cout << "SERVER: Player entity " << playerId
                                  << " has sprite with texture: " << (textureName ? textureName.value() : "NONE")
                                  << " dim: " << playerSprite.dimension.width << "x" << playerSprite.dimension.height
                                  << std::endl;
                    }

                    // Send the NEW player to all OTHER clients
                    auto network_instance = _network->getNetworkInstance();
                    if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
                        auto server = std::get<std::shared_ptr<network::Server>>(network_instance);
                        SerializationContext s_ctx = {_texture_manager};
                        auto& pools = _ecs.registry.getComponentPools();

                        uint32_t playerGuid = _ecs.registry.getConstComponent<NetworkIdentity>(playerId).guid;
                        std::cout << "SERVER: New player entity ID=" << playerId << " GUID=" << playerGuid << std::endl;

                        int componentsSent = 0;
                        for (auto& [type, pool] : pools) {
                            uint32_t typeHash = pool->getTypeHash();
                            if (_networkedComponentTypes.find(typeHash) == _networkedComponentTypes.end()) {
                                continue;
                            }

                            if (!pool->has(playerId)) {
                                continue;
                            }

                            ComponentPacket packet = pool->createPacket(playerId, s_ctx);
                            packet.entity_guid = playerGuid;

                            // Send to all OTHER clients in the lobby
                            for (const auto& client : lobby->get().getClients()) {
                                if (client.id != newClientId) {
                                    server->AddMessageToPlayer(network::GameEvents::S_SNAPSHOT, client.id, packet);
                                    componentsSent++;
                                }
                            }
                        }
                        std::cout << "SERVER: Sent " << componentsSent << " components of new player to other clients"
                                  << std::endl;
                    }

                    std::cout << "SERVER: Player entity " << newPlayer->getId() << " created for client " << newClientId
                              << std::endl;
                }
            } else {
                std::cout << "SERVER: Client " << newClientId << " FAILED to join lobby 1." << std::endl;
            }
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

    Environment env(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::SERVER);

    init();

    if (_init_function)
        _init_function(env, input_manager);

    while (1) {
        auto now = std::chrono::high_resolution_clock::now();
        ctx.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        processNetworkEvents();

        if (_loop_function) {
            _loop_function(env, input_manager);
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