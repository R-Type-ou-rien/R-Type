#include "GameManager.hpp"
#include "ECS.hpp"
#include "src/Engine/Core/LobbyState.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "src/Engine/Lib/Components/LobbyIdComponent.hpp"
#include "../../Components/spawn.hpp"
#include "../../Components/game_timer.hpp"
#include "../../Components/scripted_spawn.hpp"
#include "../../Components/pod_component.hpp"

#include "src/Engine/Lib/Components/NetworkComponents.hpp"
#include "src/RType/Common/Components/leaderboard_component.hpp"
#include <iostream>
#include <vector>
#include <string>

GameManager::GameManager() {
    try {
        _master_config = ConfigLoader::loadMasterConfig("src/RType/Common/content/config/master.cfg");
        _player_config =
            ConfigLoader::loadEntityConfig(_master_config.player_config, ConfigLoader::getRequiredPlayerFields());
        _game_config = ConfigLoader::loadGameConfig(_master_config.game_config, ConfigLoader::getRequiredGameFields());

        loadLevelFiles();

        if (!_level_files.empty()) {
            _current_level_scene = _level_files[0];
            _current_level_index = 0;
        } else {
            _current_level_scene = "src/RType/Common/content/config/levels/level1.scene";
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading master config: " << e.what() << std::endl;
        _current_level_scene = "src/RType/Common/content/config/levels/level1.scene";
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

    try {
        _current_level_config = SceneLoader::loadFromFile(_current_level_scene);

        if (!_current_level_config.game_config.empty()) {
            _game_config =
                ConfigLoader::loadGameConfig(_current_level_config.game_config, ConfigLoader::getRequiredGameFields());
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading level config: " << e.what() << std::endl;
    }

    // Client-side initialization
    if (!env->isServer()) {
        initBackground(env, _current_level_config);
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
        music.sound_name = _current_level_config.music_track.empty() ? "theme" : _current_level_config.music_track;
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
            case Environment::GameState::IN_GAME: {
                auto& ecs = env->getECS();
                std::vector<Entity> toDestroy;
                auto& sprites = ecs.registry.getEntities<sprite2D_component_s>();
                for (auto e : sprites)
                    toDestroy.push_back(e);

                auto& texts = ecs.registry.getEntities<TextComponent>();
                for (auto e : texts)
                    toDestroy.push_back(e);
                auto& audios = ecs.registry.getEntities<AudioSourceComponent>();
                for (auto e : audios)
                    toDestroy.push_back(e);

                auto& healths = ecs.registry.getEntities<HealthComponent>();
                for (auto e : healths)
                    toDestroy.push_back(e);

                std::sort(toDestroy.begin(), toDestroy.end());
                toDestroy.erase(std::unique(toDestroy.begin(), toDestroy.end()), toDestroy.end());

                for (auto e : toDestroy) {
                    ecs.registry.destroyEntity(e);
                }
                std::cout << "[GameManager] Cleaned up " << toDestroy.size() << " in-game entities" << std::endl;
            } break;
            default:
                break;
        }

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

            // Return to Lobby logic removed
            if (!env->isServer() && _windowHasFocus && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                // Logic cleared
            }
            break;
        case Environment::GameState::INCORRECT_PASSWORD:
            onAuthFailed();
            break;
        case Environment::GameState::CORRECT_PASSWORD:
            onAuthSuccess();
            break;
        case Environment::GameState::END_GAME:
            // onGameEnd(); // Removed to prevent log spam loop
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
                onServerGameStart(env, clients, lobbyId);
                _initializedLobbies.insert(lobbyId);
            } else if (state != static_cast<int>(Environment::State::IN_GAME)) {
                if (_initializedLobbies.find(lobbyId) != _initializedLobbies.end()) {
                    _initializedLobbies.erase(lobbyId);
                }
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
    _gameOver = false;
    _victory = false;
    _leaderboardDisplayed = false;
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

void GameManager::onServerGameStart(std::shared_ptr<Environment> env, const std::vector<uint32_t>& clients,
                                    uint32_t lobbyId) {
    std::cout << "GAMEMANAGER: onServerGameStart called for lobby " << lobbyId << std::endl;

    _currentLobbyId = lobbyId;

    _gameOver = false;
    _victory = false;
    _leaderboardDisplayed = false;

    auto& ecs = env->getECS();
    std::unordered_set<Entity> entitiesToDestroy;

    auto& lobbyIds = ecs.registry.getEntities<LobbyIdComponent>();
    for (auto entity : lobbyIds) {
        if (ecs.registry.hasComponent<LobbyIdComponent>(entity)) {
            if (ecs.registry.getComponent<LobbyIdComponent>(entity).lobby_id == lobbyId) {
                entitiesToDestroy.insert(entity);
            }
        }
    }

    auto& spawners = ecs.registry.getEntities<EnemySpawnComponent>();
    for (auto entity : spawners) {
        if (ecs.registry.hasComponent<EnemySpawnComponent>(entity)) {
            if (ecs.registry.getComponent<EnemySpawnComponent>(entity).lobby_id == lobbyId) {
                entitiesToDestroy.insert(entity);
            }
        }
    }

    for (auto entity : entitiesToDestroy) {
        ecs.registry.destroyEntity(entity);
    }

    LevelConfig level_config;
    try {
        level_config = SceneLoader::loadFromFile(_current_level_scene);
        if (!level_config.game_config.empty()) {
            _game_config =
                ConfigLoader::loadGameConfig(level_config.game_config, ConfigLoader::getRequiredGameFields());
        }
        _current_level_config = level_config;
    } catch (const std::exception& e) {
        std::cerr << "GAMEMANAGER: Error loading level config: " << e.what() << std::endl;
    }
    initSpawner(env, level_config);
    std::cout << "GAMEMANAGER: Server spawner entities initialized" << std::endl;

    initScene(env, level_config);
    std::cout << "GAMEMANAGER: Server scene entities initialized" << std::endl;

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

void GameManager::loadLevelFiles() {
    _level_files.clear();

    try {
        if (!_master_config.levels.empty()) {
            std::string levels_dir = _master_config.levels[0];
            if (std::filesystem::is_directory(levels_dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(levels_dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".scene") {
                        _level_files.push_back(entry.path().string());
                    }
                }

                std::sort(_level_files.begin(), _level_files.end());

                for (const auto& level : _level_files) {
                    std::cout << "  - " << level << std::endl;
                }
            } else {
                _level_files.push_back(levels_dir);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading level files: " << e.what() << std::endl;
    }
}

void GameManager::loadNextLevel(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();

    _victory = false;
    _leaderboardDisplayed = false;
    _inTransition = true;
    std::string next_level;

    if (!_current_level_config.next_level.empty()) {
        next_level = _current_level_config.next_level;
    } else {
        _current_level_index++;

        if (_current_level_index < static_cast<int>(_level_files.size())) {
            next_level = _level_files[_current_level_index];
        } else {
            _inTransition = false;
            return;
        }
    }

    if (next_level.empty()) {
        _inTransition = false;
        return;
    }

    _current_level_scene = next_level;

    auto it = std::find(_level_files.begin(), _level_files.end(), next_level);
    if (it != _level_files.end()) {
        _current_level_index = std::distance(_level_files.begin(), it);
    }

    try {
        _current_level_config = SceneLoader::loadFromFile(_current_level_scene);
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading new level config: " << e.what() << std::endl;
        return;
    }

    std::vector<Entity> leaderboard_entities;
    auto leaderboards = ecs.registry.getEntities<LeaderboardComponent>();
    for (auto entity : leaderboards) {
        leaderboard_entities.push_back(entity);
    }
    for (auto entity : leaderboard_entities) {
        std::cout << "[GameManager] Destroying leaderboard entity " << entity << std::endl;
        ecs.registry.destroyEntity(entity);
    }

    std::vector<Entity> ui_entities_to_destroy;
    auto& all_entities = ecs.registry.getEntities<TagComponent>();

    for (auto entity : all_entities) {
        if (!ecs.registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "UI" || tag == "LEADERBOARD" || tag == "VICTORY_TIMER") {
                ui_entities_to_destroy.push_back(entity);
                break;
            }
        }
    }

    for (auto entity : ui_entities_to_destroy) {
        ecs.registry.destroyEntity(entity);
    }

    auto spawners = ecs.registry.getEntities<EnemySpawnComponent>();
    for (auto spawner : spawners) {
        ecs.registry.destroyEntity(spawner);
    }

    auto scripted_spawners = ecs.registry.getEntities<ScriptedSpawnComponent>();
    for (auto spawner : scripted_spawners) {
        ecs.registry.destroyEntity(spawner);
    }

    auto pod_spawners = ecs.registry.getEntities<PodSpawnComponent>();
    for (auto spawner : pod_spawners) {
        ecs.registry.destroyEntity(spawner);
    }
    for (auto spawner : pod_spawners) {
        ecs.registry.destroyEntity(spawner);
    }

    auto timers = ecs.registry.getEntities<GameTimerComponent>();
    for (auto timer : timers) {
        ecs.registry.destroyEntity(timer);
    }

    std::vector<Entity> entities_to_destroy;
    all_entities = ecs.registry.getEntities<TagComponent>();

    int player_count = 0;
    int enemy_count = 0;
    int projectile_count = 0;
    int other_count = 0;

    for (auto entity : all_entities) {
        if (!ecs.registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
        bool should_keep = false;
        bool is_player = false;
        bool is_enemy = false;
        bool is_projectile = false;

        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                is_player = true;
                should_keep = true;
                break;
            }
            if (tag == "BACKGROUND" || tag == "BOUND") {
                should_keep = true;
                break;
            }
            if (tag == "ENEMY" || tag == "BOSS")
                is_enemy = true;
            if (tag == "PROJECTILE")
                is_projectile = true;
        }

        if (!should_keep) {
            entities_to_destroy.push_back(entity);
            if (is_player)
                player_count++;
            else if (is_enemy)
                enemy_count++;
            else if (is_projectile)
                projectile_count++;
            else
                other_count++;
        }
    }

    for (auto entity : entities_to_destroy) {
        ecs.registry.destroyEntity(entity);
    }

    std::vector<Entity> untagged_health_entities;
    auto health_entities = ecs.registry.getEntities<HealthComponent>();
    for (auto entity : health_entities) {
        if (ecs.registry.hasComponent<TagComponent>(entity))
            continue;

        bool is_player = false;
        if (ecs.registry.hasComponent<NetworkIdentity>(entity)) {
            auto& netId = ecs.registry.getConstComponent<NetworkIdentity>(entity);
            if (netId.ownerId >= 0) {
                is_player = true;
            }
        }

        if (!is_player) {
            untagged_health_entities.push_back(entity);
        }
    }

    std::cout << "[GameManager] Destroying " << untagged_health_entities.size()
              << " untagged health entities (potential invisible enemies)" << std::endl;
    for (auto entity : untagged_health_entities) {
        ecs.registry.destroyEntity(entity);
    }

    size_t total_cleaned = entities_to_destroy.size() + timers.size() + scripted_spawners.size() + pod_spawners.size() +
                           untagged_health_entities.size();
    std::cout << "[GameManager] Total cleaned: " << total_cleaned << " entities" << std::endl;

    std::cout << "[GameManager] All entities cleaned, loading new level scene..." << std::endl;
    initBackground(env, _current_level_config);
    initSpawner(env, _current_level_config);
    initScene(env, _current_level_config);

    _inTransition = false;

    std::cout << "[GameManager] ===== LEVEL " << (_current_level_index + 1) << "/" << _level_files.size()
              << " LOADED SUCCESSFULLY! =====" << std::endl;
}
