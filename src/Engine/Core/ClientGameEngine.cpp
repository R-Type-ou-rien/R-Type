#include "ClientGameEngine.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <memory>

#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/serialize/StandardComponents_serialize.hpp"
#include "Components/serialize/score_component_serialize.hpp"
#include "GameEngineBase.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "AudioSystem.hpp"

#include "../../../RType/Common/Systems/health.hpp"
#include "../../../RType/Common/Systems/score.hpp"
#include "../../../RType/Common/Components/spawn.hpp"
#include "../../../RType/Common/Components/shooter_component.hpp"
#include "../../../RType/Common/Components/charged_shot.hpp"
#include "../../../RType/Common/Components/team_component.hpp"
#include "../../../RType/Common/Components/damage_component.hpp"
#include "../../../RType/Common/Components/game_timer.hpp"
#include "../../../RType/Common/Components/pod_component.hpp"
#include "../../../RType/Common/Components/game_over_notification.hpp"
#include "../../../RType/Common/Systems/behavior.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name)
    : _window_manager(WINDOW_W, WINDOW_H, window_name),
      _env(std::make_shared<Environment>(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::CLIENT)) {
    _network = std::make_shared<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::CLIENT);
}

int ClientGameEngine::init() {
    _network->transmitEvent<int>(network::GameEvents::C_LOGIN_ANONYMOUS, 0, 0, 0);

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

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<AudioSystem>();

    // _ecs.systems.addSystem<InputSystem>(input_manager);

    // Physics and game logic handled by server - client only renders

    _ecs.systems.addSystem<PhysicsSystem>();
    // _ecs.systems.addSystem<BoxCollision>();
    // _ecs.systems.addSystem<ActionScriptSystem>();
    // _ecs.systems.addSystem<PatternSystem>();
    // _ecs.systems.addSystem<SpawnSystem>();

    // Register Network Actions (GameManager -> Engine)
    _env->addFunction("sendLogin", std::function<void(const std::string&, const std::string&)>(
                                       [this](const std::string& u, const std::string& p) { this->sendLogin(u, p); }));
    _env->addFunction("sendRegister",
                      std::function<void(const std::string&, const std::string&)>(
                          [this](const std::string& u, const std::string& p) { this->sendRegister(u, p); }));
    _env->addFunction("sendAnonymousLogin", std::function<void()>([this]() { this->sendAnonymousLogin(); }));
    _env->addFunction("createLobby",
                      std::function<void(const std::string&)>([this](const std::string& n) { this->createLobby(n); }));
    _env->addFunction("joinLobby", std::function<void(uint32_t)>([this](uint32_t id) { this->joinLobby(id); }));
    _env->addFunction("leaveLobby", std::function<void(uint32_t)>([this](uint32_t id) { this->sendLeaveLobby(id); }));
    _env->addFunction("requestLobbyList", std::function<void()>([this]() { this->requestLobbyList(); }));
    _env->addFunction("sendReady",
                      std::function<void(bool)>([this](bool r) { r ? this->sendReady() : this->sendUnready(); }));
    _env->addFunction("startGame", std::function<void(uint32_t)>([this](uint32_t) { this->sendStartGame(); }));
    _env->addFunction("sendChatMessage", std::function<void(const std::string&)>(
                                             [this](const std::string& m) { this->sendChatMessage(m); }));
    _env->addFunction("sendVoicePacket",
                      std::function<void(const engine::voice::VoicePacket&)>(
                          [this](const engine::voice::VoicePacket& p) { this->sendVoicePacket(p); }));

    // Data Access
    _env->addFunction("getLocalPlayerId", std::function<uint32_t()>([this]() { return this->getClientId(); }));
    _env->addFunction("getAvailableLobbies", std::function<std::vector<engine::core::AvailableLobby>()>(
                                                 [this]() { return this->getAvailableLobbies(); }));
    _env->addFunction("getReady", std::function<bool()>([this]() { return this->getReady(); }));
    _env->addFunction("getUnready", std::function<bool()>([this]() { return this->getUnready(); }));
    _env->addFunction("setReadyChangedCallback",
                      std::function<void(std::function<void(uint32_t, bool)>)>(
                          [this](std::function<void(uint32_t, bool)> cb) { this->setReadyChangedCallback(cb); }));
    _env->addFunction("setPlayerJoinedCallback",
                      std::function<void(std::function<void(const engine::core::LobbyPlayerInfo&)>)>(
                          [this](std::function<void(const engine::core::LobbyPlayerInfo&)> cb) {
                              this->setPlayerJoinedCallback(cb);
                          }));
    _env->addFunction("setPlayerLeftCallback",
                      std::function<void(std::function<void(uint32_t)>)>(
                          [this](std::function<void(uint32_t)> cb) { this->setPlayerLeftCallback(cb); }));
    _env->addFunction("setNewHostCallback",
                      std::function<void(std::function<void(uint32_t)>)>(
                          [this](std::function<void(uint32_t)> cb) { this->setNewHostCallback(cb); }));
    _env->addFunction(
        "setLobbyJoinedCallback",
        std::function<void(std::function<void(uint32_t, const std::string&,
                                              const std::vector<engine::core::LobbyPlayerInfo>&, uint32_t)>)>(
            [this](std::function<void(uint32_t, const std::string&, const std::vector<engine::core::LobbyPlayerInfo>&,
                                      uint32_t)>
                       cb) { this->setLobbyJoinedCallback(cb); }));
    _env->addFunction("setGameStartedCallback",
                      std::function<void(std::function<void()>)>(
                          [this](std::function<void()> cb) { this->setGameStartedCallback(cb); }));

    return 0;
}

void ClientGameEngine::handleEvent() {
    while (std::optional<sf::Event> event = _window_manager.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window_manager.getWindow().close();
        if (event->is<sf::Event::FocusLost>()) {
            input_manager.setWindowHasFocus(false);
            if (_focusChangedCallback)
                _focusChangedCallback(false);
        }
        if (event->is<sf::Event::FocusGained>()) {
            input_manager.setWindowHasFocus(true);
            if (_focusChangedCallback)
                _focusChangedCallback(true);
        }
        if (auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
            _env->pushTextInput(textEvent->unicode);
        }
    }
}

void ClientGameEngine::processNetworkEvents() {
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents();

    processLobbyEvents(pending);

    if (pending.count(network::GameEvents::S_SEND_ID)) {
        auto& msgs = pending.at(network::GameEvents::S_SEND_ID);
        for (const auto& msg : msgs) {
            auto mutable_msg = msg;
            uint32_t id;
            mutable_msg >> id;
            _clientId = id;
        }
    }

    if (pending.count(network::GameEvents::S_REGISTER_OK)) {
        _env->setGameState(Environment::GameState::CORRECT_PASSWORD);
    }
    if (pending.count(network::GameEvents::S_LOGIN_OK)) {
        _env->setGameState(Environment::GameState::CORRECT_PASSWORD);
    }
    if (pending.count(network::GameEvents::S_LOGIN_KO)) {
        _env->setGameState(Environment::GameState::INCORRECT_PASSWORD);
    }
    if (pending.count(network::GameEvents::S_INVALID_TOKEN)) {
        _env->setGameState(Environment::GameState::INCORRECT_PASSWORD);
    }
    if (pending.count(network::GameEvents::S_SNAPSHOT)) {
        auto& snapshot_packets = pending.at(network::GameEvents::S_SNAPSHOT);

        for (const auto& msg : snapshot_packets) {
            auto mutable_msg = msg;
            ComponentPacket packet;
            mutable_msg >> packet;
            processComponentPacket(packet.entity_guid, packet.component_type, packet.data, packet.owner_id);
        }
    }

    if (pending.count(network::GameEvents::S_ENTITY_DESTROY)) {
        auto& destroy_packets = pending.at(network::GameEvents::S_ENTITY_DESTROY);
        for (const auto& msg : destroy_packets) {
            auto mutable_msg = msg;
            uint32_t guid;
            mutable_msg >> guid;

            auto it = _networkToLocalEntity.find(guid);
            if (it != _networkToLocalEntity.end()) {
                Entity localId = it->second;
                _ecs.registry.destroyEntity(localId);
                _networkToLocalEntity.erase(it);
            }
        }
    }

    if (pending.count(network::GameEvents::S_ASSIGN_PLAYER_ENTITY)) {
        auto& msgs = pending.at(network::GameEvents::S_ASSIGN_PLAYER_ENTITY);
        for (auto& msg : msgs) {
            network::AssignPlayerEntityPacket packet;
            msg >> packet;
            _localPlayerEntity = packet.entityId;
        }
    }

    // Nouveau: Gérer le message S_GAME_OVER
    if (pending.count(network::GameEvents::S_GAME_OVER)) {
        auto& msgs = pending.at(network::GameEvents::S_GAME_OVER);
        for (auto& msg : msgs) {
            network::GameOverPacket packet;
            msg >> packet;

            // CRITICAL: Validate packet data before use
            if (packet.player_count > 8) {
                continue;
            }

            // Créer un composant pour signaler le game over au GameManager
            Entity gameOverEntity = _ecs.registry.createEntity();
            GameOverNotification notification;
            notification.victory = packet.victory;
            _ecs.registry.addComponent<GameOverNotification>(gameOverEntity, notification);

            // Créer des entités temporaires pour tous les joueurs avec leurs scores
            // Le GameManager pourra les lire pour afficher le leaderboard complet
            for (uint32_t i = 0; i < packet.player_count; i++) {
                Entity playerScoreEntity = _ecs.registry.createEntity();

                // Tag comme joueur pour le leaderboard
                TagComponent tags;
                tags.tags.push_back("PLAYER");
                tags.tags.push_back("LEADERBOARD_DATA");
                _ecs.registry.addComponent<TagComponent>(playerScoreEntity, tags);

                // Score du joueur
                ScoreComponent score;
                score.current_score = packet.players[i].score;
                score.high_score = 0;
                _ecs.registry.addComponent<ScoreComponent>(playerScoreEntity, score);

                // État vivant/mort
                HealthComponent health;
                health.current_hp = packet.players[i].is_alive ? 1 : 0;
                health.max_hp = 1;
                health.last_damage_time = 0;
                _ecs.registry.addComponent<HealthComponent>(playerScoreEntity, health);

                // IMPORTANT: Stocker le client_id dans NetworkIdentity pour l'affichage
                NetworkIdentity net_id;
                net_id.guid = packet.players[i].client_id;
                net_id.ownerId = packet.players[i].client_id;
                _ecs.registry.addComponent<NetworkIdentity>(playerScoreEntity, net_id);
            }
        }
    }
}

void ClientGameEngine::processLobbyEvents(
    std::map<engine::core::NetworkEngine::EventType,
             std::vector<network::message<engine::core::NetworkEngine::EventType>>>& pending) {
    // Handle ready state broadcast
    if (pending.count(network::GameEvents::S_READY_RETURN)) {
        auto& msgs = pending.at(network::GameEvents::S_READY_RETURN);
        for (auto& msg : msgs) {
            try {
                if (msg.body.size() < sizeof(uint32_t)) {
                    continue;
                }
                uint32_t clientId;
                msg >> clientId;
                _lobbyState.setPlayerReady(clientId, true);
                if (_readyChangedCallback) {
                    _readyChangedCallback(clientId, true);
                }
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT_ERROR] Error processing S_READY_RETURN: " << e.what() << std::endl;
            }
        }
    }

    // Handle lobby joined
    if (pending.count(network::GameEvents::S_ROOM_JOINED)) {
        auto& msgs = pending.at(network::GameEvents::S_ROOM_JOINED);
        for (auto& msg : msgs) {
            if (msg.body.size() < sizeof(network::lobby_in_info)) {
                continue;
            }
            network::lobby_in_info info;
            msg >> info;
            _lobbyState.lobbyId = info.id;
            _lobbyState.lobbyName = info.name;
            _lobbyState.players.clear();
            _lobbyState.hostId = info.hostId;
            _lobbyState.localClientId = _network->getClientId();
            _env->setGameState(Environment::GameState::LOBBY);
            if (_lobbyJoinedCallback) {
                _lobbyJoinedCallback(info.id, info.name, _lobbyState.players, info.hostId);
            }
        }
    }

    // Handle new host notification
    if (pending.count(network::GameEvents::S_NEW_HOST)) {
        auto& msgs = pending.at(network::GameEvents::S_NEW_HOST);
        for (auto& msg : msgs) {
            if (msg.body.size() < sizeof(uint32_t)) {
                continue;
            }
            uint32_t hostId;
            msg >> hostId;
            _lobbyState.hostId = hostId;
            for (auto& p : _lobbyState.players) {
                p.isHost = (p.id == hostId);
            }
            if (_newHostCallback)
                _newHostCallback(hostId);
        }
    }

    // Handle player joined lobby
    if (pending.count(network::GameEvents::S_PLAYER_JOINED)) {
        auto& msgs = pending.at(network::GameEvents::S_PLAYER_JOINED);
        for (auto& msg : msgs) {
            try {
                network::player playerData;
                msg >> playerData;
                engine::core::LobbyPlayerInfo newPlayer;
                newPlayer.id = playerData.id;
                newPlayer.name = playerData.username;
                newPlayer.isReady = false;
                newPlayer.isHost = (newPlayer.id == _lobbyState.hostId);

                bool found = false;
                for (auto& p : _lobbyState.players) {
                    if (p.id == newPlayer.id) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    _lobbyState.players.push_back(newPlayer);
                    if (_playerJoinedCallback)
                        _playerJoinedCallback(newPlayer);
                }
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT_ERROR] Error processing S_PLAYER_JOINED: " << e.what() << std::endl;
            }
        }
    }

    // Handle player left lobby
    if (pending.count(network::GameEvents::S_PLAYER_LEAVE)) {
        auto& msgs = pending.at(network::GameEvents::S_PLAYER_LEAVE);
        for (auto& msg : msgs) {
            if (msg.body.size() < sizeof(uint32_t)) {
                continue;
            }
            uint32_t leftPlayerId;
            msg >> leftPlayerId;
            auto it =
                std::remove_if(_lobbyState.players.begin(), _lobbyState.players.end(),
                               [leftPlayerId](const engine::core::LobbyPlayerInfo& p) { return p.id == leftPlayerId; });
            _lobbyState.players.erase(it, _lobbyState.players.end());
            if (_playerLeftCallback)
                _playerLeftCallback(leftPlayerId);
        }
    }

    // Handle unready broadcast
    if (pending.count(network::GameEvents::S_CANCEL_READY_BROADCAST)) {
        auto& msgs = pending.at(network::GameEvents::S_CANCEL_READY_BROADCAST);
        for (auto& msg : msgs) {
            if (msg.body.size() < sizeof(uint32_t)) {
                continue;
            }
            uint32_t clientId;
            msg >> clientId;
            _lobbyState.setPlayerReady(clientId, false);
            if (_readyChangedCallback)
                _readyChangedCallback(clientId, false);
        }
    }

    // Handle game start
    if (pending.count(network::GameEvents::S_GAME_START)) {
        _env->setGameState(Environment::GameState::IN_GAME);
        if (_gameStartedCallback)
            _gameStartedCallback();
    }

    // Handle game start failed
    if (pending.count(network::GameEvents::S_GAME_START_KO)) {}

    // Handle team chat messages
    if (pending.count(network::GameEvents::S_TEAM_CHAT)) {
        auto& msgs = pending.at(network::GameEvents::S_TEAM_CHAT);
        for (auto& msg : msgs) {
            try {
                network::chat_message chatMsg;
                // Read the struct (sender_id + sender_name[32] + message[256])
                if (msg.body.size() >= sizeof(network::chat_message)) {
                    std::memcpy(&chatMsg, msg.body.data() + msg.body.size() - sizeof(network::chat_message),
                                sizeof(network::chat_message));
                    msg.body.resize(msg.body.size() - sizeof(network::chat_message));
                }
                if (_env->hasFunction("receiveChatMessage")) {
                    auto func = _env->getFunction<std::function<void(const std::string&, const std::string&)>>(
                        "receiveChatMessage");
                    func(chatMsg.sender_name, chatMsg.message);
                }
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT_ERROR] Failed to process chat message: " << e.what() << std::endl;
            }
        }
    }

    // Handle voice packet relay from server
    if (pending.count(network::GameEvents::S_VOICE_RELAY)) {
        auto& msgs = pending.at(network::GameEvents::S_VOICE_RELAY);
        static uint32_t voicePacketsReceived = 0;
        for (auto& msg : msgs) {
            try {
                if (msg.body.size() >= sizeof(network::voice_packet)) {
                    network::voice_packet netPacket;
                    std::memcpy(&netPacket, msg.body.data() + msg.body.size() - sizeof(network::voice_packet),
                                sizeof(network::voice_packet));

                    engine::voice::VoicePacket packet;
                    packet.senderId = netPacket.sender_id;
                    packet.sequenceNumber = netPacket.sequence_number;
                    packet.timestamp = netPacket.timestamp;

                    if (netPacket.data_size > 0 && netPacket.data_size <= 1024) {
                        packet.encodedData.resize(netPacket.data_size);
                        std::memcpy(packet.encodedData.data(), netPacket.data, netPacket.data_size);
                    }

                    voicePacketsReceived++;

                    if (_voicePacketCallback) {
                        _voicePacketCallback(packet);
                    }
                    if (_env->hasFunction("receiveVoicePacket")) {
                        auto func = _env->getFunction<std::function<void(const engine::voice::VoicePacket&)>>(
                            "receiveVoicePacket");
                        func(packet);
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT_ERROR] Failed to process voice packet: " << e.what() << std::endl;
            }
        }
    }

    // Handle lobby list response
    if (pending.count(network::GameEvents::S_ROOMS_LIST)) {
        auto& msgs = pending.at(network::GameEvents::S_ROOMS_LIST);
        for (auto& msg : msgs) {
            try {
                if (msg.body.size() < 4) {
                    continue;
                }
                uint32_t nbLobbies = 0;
                msg >> nbLobbies;

                _availableLobbies.clear();
                for (uint32_t i = 0; i < nbLobbies; ++i) {
                    network::lobby_info info;
                    msg >> info;
                    engine::core::AvailableLobby lobby;
                    lobby.id = info.id;
                    lobby.name = info.name;
                    lobby.playerCount = info.nbConnectedPlayers;
                    lobby.maxPlayers = info.maxPlayers;
                    _availableLobbies.push_back(lobby);
                }
            } catch (const std::exception& e) {}
        }
    }

    // Handle lobby created confirmation
    if (pending.count(network::GameEvents::S_CONFIRM_NEW_LOBBY)) {
        auto& msgs = pending.at(network::GameEvents::S_CONFIRM_NEW_LOBBY);
        for (auto& msg : msgs) {
            char name[32];
            msg >> name;
            _lobbyState.lobbyName = name;
            _lobbyState.localClientId = _network->getClientId();  // Set local client ID
            _lobbyState.hostId = _network->getClientId();         // Creator is always the host
            _env->setGameState(Environment::GameState::LOBBY);

            for (auto& p : _lobbyState.players) {
                p.isHost = (p.id == _lobbyState.hostId);
            }

            if (_lobbyJoinedCallback) {
                _lobbyJoinedCallback(_lobbyState.lobbyId, _lobbyState.lobbyName, _lobbyState.players,
                                     _lobbyState.hostId);
            }
        }
    }
}

// Lobby action methods
void ClientGameEngine::sendReady() {
    if (_env->getGameState() != Environment::GameState::LOBBY) {
        return;
    }
    _network->transmitEvent<uint32_t>(network::GameEvents::C_READY, _lobbyState.lobbyId, 0, 0);
}

void ClientGameEngine::sendUnready() {
    if (_env->getGameState() != Environment::GameState::LOBBY) {
        return;
    }
    _network->transmitEvent<uint32_t>(network::GameEvents::C_CANCEL_READY, _lobbyState.lobbyId, 0, 0);
}

void ClientGameEngine::sendStartGame() {
    if (_env->getGameState() != Environment::GameState::LOBBY) {
        return;
    }
    if (!_lobbyState.isLocalPlayerHost()) {
        return;
    }
    _network->transmitEvent<uint32_t>(network::GameEvents::C_GAME_START, _lobbyState.lobbyId, 0, 0);
}

void ClientGameEngine::createLobby(const std::string& name) {
    char lobbyName[32] = {0};
    std::strncpy(lobbyName, name.c_str(), 31);
    _network->transmitEvent<char[32]>(network::GameEvents::C_NEW_LOBBY, lobbyName, 0, 0);
}

void ClientGameEngine::joinLobby(uint32_t lobbyId) {
    _network->transmitEvent<uint32_t>(network::GameEvents::C_JOIN_ROOM, lobbyId, 0, 0);
}

void ClientGameEngine::sendLeaveLobby(uint32_t lobbyId) {
    _network->transmitEvent<uint32_t>(network::GameEvents::C_ROOM_LEAVE, lobbyId, 0, 0);
}

void ClientGameEngine::sendChatMessage(const std::string& message) {
    if (_env->getGameState() != Environment::GameState::LOBBY) {
        return;
    }
    char msgBuffer[256] = {0};
    std::strncpy(msgBuffer, message.c_str(), 255);
    _network->transmitEvent<char[256]>(network::GameEvents::C_TEAM_CHAT, msgBuffer, 0, 0);
}

void ClientGameEngine::sendVoicePacket(const engine::voice::VoicePacket& packet) {
    if (_env->getGameState() != Environment::GameState::LOBBY) {
        return;
    }
    // Use fixed-size struct for network transmission
    network::voice_packet netPacket;
    std::memset(&netPacket, 0, sizeof(netPacket));

    netPacket.sender_id = packet.senderId;
    netPacket.sequence_number = packet.sequenceNumber;
    netPacket.timestamp = packet.timestamp;
    netPacket.data_size = std::min(static_cast<uint32_t>(packet.encodedData.size()), 1024u);

    if (netPacket.data_size > 0) {
        std::memcpy(netPacket.data, packet.encodedData.data(), netPacket.data_size);
    }

    _network->transmitEvent<network::voice_packet>(network::GameEvents::C_VOICE_PACKET, netPacket, 0, 0);
}

void ClientGameEngine::requestLobbyList() {
    _network->transmitEvent<uint32_t>(network::GameEvents::C_LIST_ROOMS, 0, 0, 0);
}

void ClientGameEngine::sendLogin(const std::string& username, const std::string& password) {
    network::connection_info info;
    std::memset(&info, 0, sizeof(info));
    std::strncpy(info.username, username.c_str(), 31);
    std::strncpy(info.password, password.c_str(), 31);
    _network->transmitEvent<network::connection_info>(network::GameEvents::C_LOGIN, info, 0, 0);
}

void ClientGameEngine::sendRegister(const std::string& username, const std::string& password) {
    network::connection_info info;
    std::memset(&info, 0, sizeof(info));
    std::strncpy(info.username, username.c_str(), 31);
    std::strncpy(info.password, password.c_str(), 31);
    _network->transmitEvent<network::connection_info>(network::GameEvents::C_REGISTER, info, 0, 0);
}

void ClientGameEngine::sendAnonymousLogin() {
    _network->transmitEvent<int>(network::GameEvents::C_LOGIN_ANONYMOUS, 0, 0, 0);
}

int ClientGameEngine::run() {
    system_context context = {0,
                              _currentTick,
                              _texture_manager,
                              _sound_manager,
                              _music_manager,
                              _window_manager.getWindow(),
                              input_manager,
                              _clientId};
    auto last_time = std::chrono::high_resolution_clock::now();

    this->init();

    if (_init_function)
        _init_function(_env, input_manager);

    while (_window_manager.isOpen()) {
        auto now = std::chrono::high_resolution_clock::now();
        context.dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0f;
        last_time = now;

        if (context.player_id == 0)
            context.player_id = _clientId;

        handleEvent();
        processNetworkEvents();
        input_manager.update(*_network, _currentTick, context);
        _window_manager.clear();

        if (_loop_function)
            _loop_function(_env, input_manager);
        _ecs.update(context);

        _window_manager.display();
        _currentTick++;
    }
    return SUCCESS;
}
