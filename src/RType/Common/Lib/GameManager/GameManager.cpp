#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "Lobby.hpp"
#include "Network.hpp"
#include "src/Engine/Core/ClientGameEngine.hpp"
#include "src/Engine/Core/LobbyState.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"
#include <cmath>
#include <cstdint>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "../../Systems/score.hpp"
#include "../../Components/spawn.hpp"
#include "../../Systems/spawn.hpp"
#include "../../Components/shooter_component.hpp"
#include "../../Components/charged_shot.hpp"
#include "../../Components/team_component.hpp"
#include "../../Components/damage_component.hpp"
#include "../../Components/game_timer.hpp"
#include "../../Components/pod_component.hpp"
#include "../../Components/scripted_spawn.hpp"
#include "../../Systems/behavior.hpp"
#include "src/Engine/Lib/Systems/PhysicsSystem.hpp"
#include "src/Engine/Lib/Systems/CollisionSystem.hpp"

GameManager::GameManager() {
    try {
        _master_config = ConfigLoader::loadMasterConfig("src/RType/Common/content/config/master.cfg");
        _player_config =
            ConfigLoader::loadEntityConfig(_master_config.player_config, ConfigLoader::getRequiredPlayerFields());
        _game_config = ConfigLoader::loadGameConfig(_master_config.game_config, ConfigLoader::getRequiredGameFields());
        if (!_master_config.levels.empty()) {
            _current_level_scene = _master_config.levels[0];
        } else {
            _current_level_scene = "src/RType/Common/content/config/level1.scene";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading master config: " << e.what() << std::endl;
        _current_level_scene = "src/RType/Common/content/config/level1.scene";
    }

    _gameOver = false;
    _victory = false;
    _leaderboardDisplayed = false;

    _authManager = std::make_unique<AuthManager>();
    _menuManager = std::make_unique<MenuManager>();
    _lobbyManager = std::make_shared<LobbyManager>();

#ifdef CLIENT_BUILD
    // Initialize voice manager
    _voiceManager = std::make_unique<engine::voice::VoiceManager>();
    engine::voice::VoiceConfig voiceConfig;
    voiceConfig.sampleRate = 48000;
    voiceConfig.framesPerBuffer = 960;  // 20ms at 48kHz
    voiceConfig.channels = 1;
    _voiceManager->setConfig(voiceConfig);
    std::cout << "[VoiceManager] Created" << std::endl;

    _lobbyManager->setVoiceManager(_voiceManager.get());
#endif
}

void GameManager::init(std::shared_ptr<Environment> env, InputManager& inputs) {
    _env = env;
    initSystems(_env);
    if (!_master_config.resources_config.empty()) {
        _env->loadGameResources(_master_config.resources_config);
    } else {
        _env->loadGameResources("src/RType/Common/content/config/r-type.json");
    }

    if (!_env->isServer()) {
        _env->setGameState(Environment::GameState::AUTHSCREEN);
        _authManager->init(_env);
        _previousState = Environment::GameState::AUTHSCREEN;
    } else {
        _env->setGameState(Environment::GameState::SERVER);
        _previousState = Environment::GameState::SERVER;
    }

    if (env->hasFunction("setReadyChangedCallback")) {
        auto setter =
            env->getFunction<std::function<void(std::function<void(uint32_t, bool)>)>>("setReadyChangedCallback");
        setter([this](uint32_t id, bool ready) { this->onPlayerReadyChanged(id, ready); });
    }
    if (env->hasFunction("setPlayerJoinedCallback")) {
        auto setter = env->getFunction<std::function<void(std::function<void(const engine::core::LobbyPlayerInfo&)>)>>(
            "setPlayerJoinedCallback");
        setter([this](const engine::core::LobbyPlayerInfo& p) {
            LobbyPlayerInfo player;
            player.id = p.id;
            player.name = p.name;
            player.isReady = p.isReady;
            player.isHost = p.isHost;
            this->onPlayerJoined(player);
        });
    }
    if (env->hasFunction("setPlayerLeftCallback")) {
        auto setter = env->getFunction<std::function<void(std::function<void(uint32_t)>)>>("setPlayerLeftCallback");
        setter([this](uint32_t id) { this->onPlayerLeft(id); });
    }
    if (env->hasFunction("setNewHostCallback")) {
        auto setter = env->getFunction<std::function<void(std::function<void(uint32_t)>)>>("setNewHostCallback");
        setter([this](uint32_t id) { this->onNewHost(id); });
    }
    if (env->hasFunction("setLobbyJoinedCallback")) {
        auto setter = env->getFunction<
            std::function<void(std::function<void(uint32_t, const std::string&,
                                                  const std::vector<engine::core::LobbyPlayerInfo>&, uint32_t)>)>>(
            "setLobbyJoinedCallback");
        setter([this](uint32_t id, const std::string& name, const std::vector<engine::core::LobbyPlayerInfo>& players,
                      uint32_t hostId) {
            std::vector<LobbyPlayerInfo> lobbyPlayers;
            for (const auto& p : players) {
                LobbyPlayerInfo info;
                info.id = p.id;
                info.name = p.name;
                info.isReady = p.isReady;
                info.isHost = p.isHost;
                lobbyPlayers.push_back(info);
            }
            this->onLobbyJoined(id, name, lobbyPlayers, hostId);
        });
    }
    if (env->hasFunction("setGameStartedCallback")) {
        auto setter = env->getFunction<std::function<void(std::function<void()>)>>("setGameStartedCallback");
        setter([this]() { this->onGameStarted(); });
    }
}

void GameManager::startGame(std::shared_ptr<Environment> env, InputManager& inputs) {
    if (_gameInitialized)
        return;

    LevelConfig level_config;
    try {
        level_config = SceneLoader::loadFromFile(_current_level_scene);
        if (!level_config.game_config.empty()) {
            _game_config =
                ConfigLoader::loadGameConfig(level_config.game_config, ConfigLoader::getRequiredGameFields());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading master config: " << e.what() << std::endl;
    }

    // Client-side initialization
    if (!env->isServer()) {
        initBackground(env, level_config);
        initBounds(env);
        initPlayer(env);  // This inits local player components/input handling
        initUI(env);
    } else {
        // Server initialization is now handled in updateServer -> onServerGameStart
        return;
    }

    if (!env->isServer()) {
        auto& ecs = env->getECS();
        Entity musicEntity = ecs.registry.createEntity();
        AudioSourceComponent music;
        music.sound_name = level_config.music_track.empty() ? "theme" : level_config.music_track;
        music.play_on_start = true;
        music.loop = true;
        music.destroy_entity_on_finish = false;
        ecs.registry.addComponent<AudioSourceComponent>(musicEntity, music);
        loadInputSetting(inputs);
    }

    _gameInitialized = true;
    _env->setGameState(Environment::GameState::IN_GAME);
    return;
}

void GameManager::update(std::shared_ptr<Environment> env, InputManager& inputs) {
    checkGameState(env);

    if (_pendingGameStart) {
        _pendingGameStart = false;
        std::cout << "[GameManager] Processing pending game start - initializing game scene" << std::endl;

        if (_previousState == Environment::GameState::LOBBY) {
            _lobbyManager->cleanupLobby(env);
        }

        LevelConfig level_config;
        try {
            level_config = SceneLoader::loadFromFile(_current_level_scene);
            if (!level_config.game_config.empty()) {
                _game_config =
                    ConfigLoader::loadGameConfig(level_config.game_config, ConfigLoader::getRequiredGameFields());
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading master config: " << e.what() << std::endl;
        }

        initBackground(env, level_config);
        initBounds(env);
        std::cout << "[GameManager] Multiplayer mode: Player spawning handled by network replication" << std::endl;
        initSpawner(env, level_config);
        initScene(env, level_config);
        initUI(env);

        if (!env->isServer()) {
            auto& ecs = env->getECS();
            Entity musicEntity = ecs.registry.createEntity();
            AudioSourceComponent music;
            music.sound_name = level_config.music_track.empty() ? "theme" : level_config.music_track;
            music.play_on_start = true;
            music.loop = true;
            music.destroy_entity_on_finish = false;
            ecs.registry.addComponent<AudioSourceComponent>(musicEntity, music);
            loadInputSetting(inputs);
        }

        _gameInitialized = true;
        env->setGameState(Environment::GameState::IN_GAME);
        _previousState = Environment::GameState::IN_GAME;
    }

    auto currentState = env->getGameState();

    if (currentState != _previousState) {
        // Cleanup
        switch (_previousState) {
            case Environment::GameState::AUTHSCREEN:
                _authManager->cleanup(env);
                break;
            case Environment::GameState::MAIN_MENU:
                _menuManager->cleanup(env);
                break;
            case Environment::GameState::LOBBY_LIST:
                _lobbyManager->cleanupBrowser(env);
                break;
            case Environment::GameState::LOBBY:
                _lobbyManager->cleanupLobby(env);
                break;
            default:
                break;
        }

        // Init
        switch (currentState) {
            case Environment::GameState::AUTHSCREEN:
                _authManager->init(env);
                break;
            case Environment::GameState::MAIN_MENU:
                _menuManager->init(env);
                break;
            case Environment::GameState::LOBBY_LIST:
                _lobbyManager->initBrowser(env);
                break;
            case Environment::GameState::LOBBY:
                if (env->hasFunction("getLocalPlayerId")) {
                    auto func = env->getFunction<std::function<uint32_t()>>("getLocalPlayerId");
                    _localPlayerId = func();
                }
                _lobbyManager->initLobby(env, _lobbyManager->getCurrentLobbyId(), _lobbyManager->getCurrentLobbyName(),
                                         _localPlayerId);
                break;
            default:
                break;
        }
        _previousState = currentState;
    }

    // Update
    switch (currentState) {
        case Environment::GameState::AUTHSCREEN:
            _authManager->update(env, _window, _windowHasFocus);
            break;
        case Environment::GameState::MAIN_MENU:
            _menuManager->update(env, _window, _windowHasFocus);
            break;
        case Environment::GameState::LOBBY_LIST:

            _lobbyManager->updateBrowser(env, _window, _windowHasFocus, _localPlayerId);
            break;
        case Environment::GameState::LOBBY:

            _lobbyManager->updateLobby(env, _window, _windowHasFocus, _localPlayerId);
            break;
        case Environment::GameState::IN_GAME:
            updateUI(env);
            checkGameState(env);
            break;
        case Environment::GameState::INCORRECT_PASSWORD:
            onAuthFailed();
            break;
        case Environment::GameState::CORRECT_PASSWORD:
            onAuthSuccess();
            break;
        case Environment::GameState::END_GAME:
            onGameEnd();
            break;
        case Environment::GameState::SERVER:
            updateServer(env, inputs);
            break;
    }
}

void GameManager::updateServer(std::shared_ptr<Environment> env, InputManager& inputs) {
    if (env->hasFunction("forEachLobby")) {
        auto forEachLobby =
            env->getFunction<std::function<void(std::function<void(uint32_t, int, const std::vector<uint32_t>&)>)>>(
                "forEachLobby");

        forEachLobby([this, &env](uint32_t lobbyId, int state, const std::vector<uint32_t>& clients) {
            if (state == static_cast<int>(Environment::State::IN_GAME) &&
                _initializedLobbies.find(lobbyId) == _initializedLobbies.end()) {
                std::cout << "GAME MANAGER: Lobby " << lobbyId << " started game. Initializing..." << std::endl;
                onServerGameStart(env, clients);
                _initializedLobbies.insert(lobbyId);
            }
        });
    }
}

void GameManager::onAuthSuccess() {
    if (_env && _env->hasFunction("getLocalPlayerId")) {
        auto func = _env->getFunction<std::function<uint32_t()>>("getLocalPlayerId");
        setLocalPlayerId(func());
        std::cout << "[GameManager] Auth success, local ID set to: " << _localPlayerId << std::endl;
    }
    _env->setGameState(Environment::GameState::MAIN_MENU);
}

void GameManager::onAuthFailed() {
    _authManager->setAuthFailed();
    _env->setGameState(Environment::GameState::AUTHSCREEN);
}

void GameManager::onLobbyListReceived(const std::vector<LobbyInfo>& lobbies) {
    _lobbyManager->setAvailableLobbies(lobbies);
}

void GameManager::onLobbyJoined(uint32_t lobbyId, const std::string& name, const std::vector<LobbyPlayerInfo>& players,
                                uint32_t hostId) {
    _lobbyManager->setPlayersInLobby(players);
    _lobbyManager->setHostId(hostId);
}

void GameManager::onPlayerJoined(const LobbyPlayerInfo& player) {
    _lobbyManager->addPlayer(player);
}

void GameManager::onPlayerLeft(uint32_t playerId) {
    auto& players = _lobbyManager->getPlayersInLobby();
    players.erase(std::remove_if(players.begin(), players.end(),
                                 [playerId](const LobbyPlayerInfo& p) { return p.id == playerId; }),
                  players.end());
}

void GameManager::onPlayerReadyChanged(uint32_t playerId, bool ready) {
    _lobbyManager->setPlayerReady(playerId, ready);
}

void GameManager::onNewHost(uint32_t hostId) {
    _lobbyManager->setHostId(hostId);
}

void GameManager::onGameStarted() {
    std::cout << "[GameManager] onGameStarted called, setting pending game start flag" << std::endl;
    _pendingGameStart = true;
}

void GameManager::onGameEnd() {
    std::cout << "[GameManager] onGameEnd called, setting pending game end flag" << std::endl;
    _env->setGameState(Environment::GameState::END_GAME);
}

void GameManager::onVoicePacketReceived(const engine::voice::VoicePacket& packet) {
#ifdef CLIENT_BUILD
    if (_voiceManager) {
        _voiceManager->receivePacket(packet);
    }
#endif
}

void GameManager::onChatMessageReceived(const std::string& senderName, const std::string& message) {
    _lobbyManager->onChatMessageReceived(senderName, message);
}

void GameManager::onServerGameStart(std::shared_ptr<Environment> env, const std::vector<uint32_t>& clients) {
    std::cout << "GAMEMANAGER: onServerGameStart called" << std::endl;

    static bool systemsInitialized = false;
    if (!systemsInitialized) {
        std::cout << "GAMEMANAGER: Initializing SERVER game systems..." << std::endl;
        auto& ecs = env->getECS();

        ecs.systems.addSystem<BoxCollision>();
        ecs.systems.addSystem<EnemySpawnSystem>();
        ecs.systems.addSystem<PhysicsSystem>();
        std::cout << "GAMEMANAGER: Server game systems initialized" << std::endl;

        Entity spawnerEntity = ecs.registry.createEntity();
        EnemySpawnComponent spawnComp;
        spawnComp.spawn_interval = 2.0f;
        spawnComp.is_active = true;
        spawnComp.use_scripted_spawns = false;
        spawnComp.enemies_config_path = "src/RType/Common/content/config/enemies.cfg";
        spawnComp.boss_config_path = "src/RType/Common/content/config/boss.cfg";
        spawnComp.game_config_path = "src/RType/Common/content/config/game.cfg";
        ecs.registry.addComponent<EnemySpawnComponent>(spawnerEntity, spawnComp);
        ecs.registry.addComponent<NetworkIdentity>(spawnerEntity, {spawnerEntity, 0});

        Entity scriptedSpawnerEntity = ecs.registry.createEntity();
        ScriptedSpawnComponent scriptedSpawnComp;
        scriptedSpawnComp.script_path = "src/RType/Common/content/config/level1_spawns.cfg";
        ecs.registry.addComponent<ScriptedSpawnComponent>(scriptedSpawnerEntity, scriptedSpawnComp);
        ecs.registry.addComponent<NetworkIdentity>(scriptedSpawnerEntity, {scriptedSpawnerEntity, 0});

        Entity timerEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<GameTimerComponent>(timerEntity, {0.0f});
        ecs.registry.addComponent<NetworkIdentity>(timerEntity, {timerEntity, 0});

        systemsInitialized = true;
    }

    if (!env->hasFunction("registerPlayer")) {
        std::cerr << "GAMEMANAGER: Error - registerPlayer hook not found!" << std::endl;
        return;
    }
    auto registerFunc = env->getFunction<std::function<void(uint32_t, std::shared_ptr<Player>)>>("registerPlayer");

    size_t playerIndex = 0;
    for (uint32_t clientId : clients) {
        float startX = 100.0f;
        float startY = 100.0f + (playerIndex * 100.0f);

        auto newPlayer = createPlayerForClient(env, clientId, startX, startY);
        if (newPlayer) {
            registerFunc(clientId, newPlayer);
            std::cout << "GAMEMANAGER: Created and registered player for client " << clientId << std::endl;
        }
        playerIndex++;
    }
}
