#include "ClientGameEngine.hpp"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>

#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "Network.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
#include "AudioSystem.hpp"

#include "../../../RType/Common/Systems/health.hpp"
#include "../../../RType/Common/Components/spawn.hpp"
#include "../../../RType/Common/Components/shooter_component.hpp"
#include "../../../RType/Common/Components/charged_shot.hpp"
#include "../../../RType/Common/Components/team_component.hpp"
#include "../../../RType/Common/Components/damage_component.hpp"
#include "../../../RType/Common/Components/game_timer.hpp"
#include "../../../RType/Common/Components/pod_component.hpp"
#include "../../../RType/Common/Systems/ai_behavior.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name) {
    _network = std::make_unique<engine::core::NetworkEngine>(engine::core::NetworkEngine::NetworkRole::CLIENT);
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
    registerNetworkComponent<AIBehaviorComponent>();
    registerNetworkComponent<BossComponent>();

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<AudioSystem>();
    //_ecs.systems.addSystem<InputSystem>(input_manager);

    // Physics and game logic handled by server - client only renders
    _ecs.systems.addSystem<PhysicsSystem>();
    // _ecs.systems.addSystem<BoxCollision>();
    // _ecs.systems.addSystem<ActionScriptSystem>();
    // _ecs.systems.addSystem<PatternSystem>();
    // _ecs.systems.addSystem<SpawnSystem>();
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
    }
}

void ClientGameEngine::processNetworkEvents() {
    _network->processIncomingPackets(_currentTick);
    auto pending = _network->getPendingEvents();

    // Process lobby events first
    // Process lobby events first
    // Process lobby events first
    processLobbyEvents(pending);

    if (pending.count(network::GameEvents::S_REGISTER_OK)) {
        std::cout << "[CLIENT] Registration success!" << std::endl;
        if (_authSuccessCallback)
            _authSuccessCallback();
    }
    if (pending.count(network::GameEvents::S_LOGIN_OK)) {
        std::cout << "[CLIENT] Login success!" << std::endl;
        if (_authSuccessCallback)
            _authSuccessCallback();
    }
    if (pending.count(network::GameEvents::S_LOGIN_KO)) {
        std::cout << "[CLIENT] Login failed!" << std::endl;
        if (_authFailedCallback)
            _authFailedCallback();
    }
    if (pending.count(network::GameEvents::S_INVALID_TOKEN)) {
        std::cout << "[CLIENT] Invalid token, please login again." << std::endl;
        if (_authFailedCallback)
            _authFailedCallback();
    }

    if (pending.count(network::GameEvents::S_SNAPSHOT)) {
        auto& snapshot_packets = pending.at(network::GameEvents::S_SNAPSHOT);

        for (const auto& msg : snapshot_packets) {
            auto mutable_msg = msg;
            ComponentPacket packet;
            mutable_msg >> packet;
            processComponentPacket(packet.entity_guid, packet.component_type, packet.data);
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
                std::cout << "[CLIENT] Destroyed entity guid=" << guid << " localId=" << localId << std::endl;
            }
        }
    }

    if (pending.count(network::GameEvents::S_VOICE_RELAY)) {
        std::cout << "Vocal replay not implemented" << std::endl;
    }

    if (pending.count(network::GameEvents::S_ASSIGN_PLAYER_ENTITY)) {
        auto& msgs = pending.at(network::GameEvents::S_ASSIGN_PLAYER_ENTITY);
        for (auto& msg : msgs) {
            network::AssignPlayerEntityPacket packet;
            msg >> packet;
            _localPlayerEntity = packet.entityId;
        }
    }
}

void ClientGameEngine::processLobbyEvents(
    std::map<engine::core::NetworkEngine::EventType,
             std::vector<network::message<engine::core::NetworkEngine::EventType>>>& pending) {
    // Handle lobby joined
    if (pending.count(network::GameEvents::S_ROOM_JOINED)) {
        auto& msgs = pending.at(network::GameEvents::S_ROOM_JOINED);
        for (auto& msg : msgs) {
            network::lobby_in_info info;
            msg >> info;
            _lobbyState.lobbyId = info.id;
            _lobbyState.lobbyName = info.name;
            _lobbyState.players.clear();
            _lobbyState.hostId = info.hostId;
            _lobbyState.localClientId = _network->getClientId();  // Set local client ID
            // Players will be populated by S_PLAYER_JOINED separately, or we can trust S_ROOM_JOINED's size?
            // Wait, S_ROOM_JOINED contains ID list. We should probably use it if S_PLAYER_JOINED is not guaranteed for
            // initial state? But we decided to handle S_PLAYER_JOINED.
            _currentScene = GameScene::LOBBY;
            std::cout << "[CLIENT] Joined lobby " << info.id << " (" << info.name << ") with " << info.nbPlayers
                      << " players" << std::endl;
            if (_lobbyJoinedCallback) {
                _lobbyJoinedCallback(info.id, info.name, _lobbyState.players, info.hostId);
            }
        }
    }

    // Handle new host notification
    if (pending.count(network::GameEvents::S_NEW_HOST)) {
        auto& msgs = pending.at(network::GameEvents::S_NEW_HOST);
        for (auto& msg : msgs) {
            uint32_t hostId;
            msg >> hostId;
            _lobbyState.hostId = hostId;
            for (auto& p : _lobbyState.players) {
                p.isHost = (p.id == hostId);
            }
            std::cout << "[CLIENT] New host: " << hostId << (hostId == _clientId ? " (that's me!)" : "") << std::endl;
            if (_newHostCallback)
                _newHostCallback(hostId);
        }
    }

    // Handle player joined lobby
    if (pending.count(network::GameEvents::S_PLAYER_JOINED)) {
        auto& msgs = pending.at(network::GameEvents::S_PLAYER_JOINED);
        for (auto& msg : msgs) {
            try {
                std::cout << "[CLIENT_DEBUG] Processing S_PLAYER_JOINED. Body size: " << msg.body.size() << std::endl;
                network::player playerData;
                msg >> playerData;
                engine::core::LobbyPlayerInfo newPlayer;
                newPlayer.id = playerData.id;
                newPlayer.name = playerData.username;
                newPlayer.isReady = false;
                newPlayer.isHost = false;
                _lobbyState.players.push_back(newPlayer);
                std::cout << "[CLIENT] Player " << playerData.username << " (" << playerData.id << ") joined lobby"
                          << std::endl;
                if (_playerJoinedCallback)
                    _playerJoinedCallback(newPlayer);
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT_ERROR] Failed to process S_PLAYER_JOINED: " << e.what() << std::endl;
            }
        }
    }

    // Handle player left lobby
    if (pending.count(network::GameEvents::S_PLAYER_LEAVE)) {
        auto& msgs = pending.at(network::GameEvents::S_PLAYER_LEAVE);
        for (auto& msg : msgs) {
            uint32_t leftPlayerId;
            msg >> leftPlayerId;
            auto it =
                std::remove_if(_lobbyState.players.begin(), _lobbyState.players.end(),
                               [leftPlayerId](const engine::core::LobbyPlayerInfo& p) { return p.id == leftPlayerId; });
            _lobbyState.players.erase(it, _lobbyState.players.end());
            std::cout << "[CLIENT] Player " << leftPlayerId << " left lobby" << std::endl;
            if (_playerLeftCallback)
                _playerLeftCallback(leftPlayerId);
        }
    }

    // Handle ready state broadcast
    if (pending.count(network::GameEvents::S_READY_RETURN)) {
        auto& msgs = pending.at(network::GameEvents::S_READY_RETURN);
        for (auto& msg : msgs) {
            uint32_t clientId;
            msg >> clientId;
            _lobbyState.setPlayerReady(clientId, true);
            std::cout << "[CLIENT] Player " << clientId << " is ready" << std::endl;
            if (_readyChangedCallback)
                _readyChangedCallback(clientId, true);
        }
    }

    // Handle unready broadcast
    if (pending.count(network::GameEvents::S_CANCEL_READY_BROADCAST)) {
        auto& msgs = pending.at(network::GameEvents::S_CANCEL_READY_BROADCAST);
        for (auto& msg : msgs) {
            uint32_t clientId;
            msg >> clientId;
            _lobbyState.setPlayerReady(clientId, false);
            std::cout << "[CLIENT] Player " << clientId << " is not ready" << std::endl;
            if (_readyChangedCallback)
                _readyChangedCallback(clientId, false);
        }
    }

    // Handle game start
    if (pending.count(network::GameEvents::S_GAME_START)) {
        _currentScene = GameScene::IN_GAME;
        std::cout << "[CLIENT] Game starting!" << std::endl;
        if (_gameStartedCallback)
            _gameStartedCallback();
    }

    // Handle game start failed
    if (pending.count(network::GameEvents::S_GAME_START_KO)) {
        std::cout << "[CLIENT] Game start failed - not all players ready or not host" << std::endl;
    }

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
                std::cout << "[CLIENT] Chat from " << chatMsg.sender_name << ": " << chatMsg.message << std::endl;
                if (_chatMessageCallback)
                    _chatMessageCallback(chatMsg.sender_name, chatMsg.message);
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT_ERROR] Failed to process chat message: " << e.what() << std::endl;
            }
        }
    }

    // Handle lobby list response
    if (pending.count(network::GameEvents::S_ROOMS_LIST)) {
        auto& msgs = pending.at(network::GameEvents::S_ROOMS_LIST);
        for (auto& msg : msgs) {
            try {
                std::cout << "[CLIENT_DEBUG] Processing S_ROOMS_LIST. Body size: " << msg.body.size() << std::endl;
                if (msg.body.size() < 4) {
                    std::cerr << "[CLIENT_ERROR] S_ROOMS_LIST body too small (" << msg.body.size() << ")!" << std::endl;
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
                std::cout << "[CLIENT] Received " << nbLobbies << " lobbies from server" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[CLIENT] Error processing lobby list: " << e.what() << std::endl;
            }
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
            _currentScene = GameScene::LOBBY;
            std::cout << "[CLIENT] Lobby '" << name << "' created successfully (hostId=" << _lobbyState.hostId
                      << ", localId=" << _lobbyState.localClientId << ")" << std::endl;
        }
    }
}

// Lobby action methods
void ClientGameEngine::sendReady() {
    if (_currentScene != GameScene::LOBBY)
        return;
    _network->transmitEvent<uint32_t>(network::GameEvents::C_READY, _lobbyState.lobbyId, 0, 0);
    std::cout << "[CLIENT] Sending ready" << std::endl;
}

void ClientGameEngine::sendUnready() {
    if (_currentScene != GameScene::LOBBY)
        return;
    _network->transmitEvent<uint32_t>(network::GameEvents::C_CANCEL_READY, _lobbyState.lobbyId, 0, 0);
    std::cout << "[CLIENT] Sending unready" << std::endl;
}

void ClientGameEngine::sendStartGame() {
    if (_currentScene != GameScene::LOBBY)
        return;
    if (!_lobbyState.isLocalPlayerHost()) {
        std::cout << "[CLIENT] Cannot start game - not host" << std::endl;
        return;
    }
    _network->transmitEvent<uint32_t>(network::GameEvents::C_GAME_START, _lobbyState.lobbyId, 0, 0);
    std::cout << "[CLIENT] Sending start game" << std::endl;
}

void ClientGameEngine::createLobby(const std::string& name) {
    char lobbyName[32] = {0};
    std::strncpy(lobbyName, name.c_str(), 31);
    _network->transmitEvent<char[32]>(network::GameEvents::C_NEW_LOBBY, lobbyName, 0, 0);
    std::cout << "[CLIENT] Creating lobby: " << name << std::endl;
}

void ClientGameEngine::joinLobby(uint32_t lobbyId) {
    _network->transmitEvent<uint32_t>(network::GameEvents::C_JOIN_ROOM, lobbyId, 0, 0);
    std::cout << "[CLIENT] Joining lobby: " << lobbyId << std::endl;
    std::cout << "[CLIENT] Joining lobby: " << lobbyId << std::endl;
}

void ClientGameEngine::sendLeaveLobby(uint32_t lobbyId) {
    _network->transmitEvent<uint32_t>(network::GameEvents::C_ROOM_LEAVE, lobbyId, 0, 0);
    std::cout << "[CLIENT] Leaving lobby: " << lobbyId << std::endl;
}

void ClientGameEngine::sendChatMessage(const std::string& message) {
    if (_currentScene != GameScene::LOBBY)
        return;
    char msgBuffer[256] = {0};
    std::strncpy(msgBuffer, message.c_str(), 255);
    _network->transmitEvent<char[256]>(network::GameEvents::C_TEAM_CHAT, msgBuffer, 0, 0);
    std::cout << "[CLIENT] Sending chat message: " << message << std::endl;
}

void ClientGameEngine::requestLobbyList() {
    _network->transmitEvent<uint32_t>(network::GameEvents::C_LIST_ROOMS, 0, 0, 0);
    std::cout << "[CLIENT] Requesting lobby list from server" << std::endl;
}

void ClientGameEngine::sendLogin(const std::string& username, const std::string& password) {
    network::connection_info info;
    std::memset(&info, 0, sizeof(info));  // Ensure clean struct
    std::strncpy(info.username, username.c_str(), 31);
    std::strncpy(info.password, password.c_str(), 31);
    _network->transmitEvent<network::connection_info>(network::GameEvents::C_LOGIN, info, 0, 0);
    std::cout << "[CLIENT] Sending login for " << username << std::endl;
}

void ClientGameEngine::sendRegister(const std::string& username, const std::string& password) {
    network::connection_info info;
    std::memset(&info, 0, sizeof(info));
    std::strncpy(info.username, username.c_str(), 31);
    std::strncpy(info.password, password.c_str(), 31);
    _network->transmitEvent<network::connection_info>(network::GameEvents::C_REGISTER, info, 0, 0);
    std::cout << "[CLIENT] Sending register for " << username << std::endl;
}

void ClientGameEngine::sendAnonymousLogin() {
    _network->transmitEvent<int>(network::GameEvents::C_LOGIN_ANONYMOUS, 0, 0, 0);
    std::cout << "[CLIENT] Sending anonymous login" << std::endl;
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

    Environment env(_ecs, _texture_manager, _sound_manager, _music_manager, EnvMode::CLIENT);

    this->init();
    if (_init_function)
        _init_function(env, input_manager);

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
            _loop_function(env, input_manager);
        _ecs.update(context);

        _window_manager.display();
        _currentTick++;
    }
    return SUCCESS;
}
