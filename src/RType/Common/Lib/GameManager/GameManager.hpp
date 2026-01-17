#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../../Entities/Player/Player.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/RType/Common/Components/config.hpp"
#include "src/RType/Common/Components/game_timer.hpp"
#include "src/Engine/Core/Scene/SceneManager.hpp"
#include "Voice/VoiceManager.hpp"

// Include new managers
#include "auth/AuthManager.hpp"
#include "menu/MenuManager.hpp"
#include "lobby/LobbyManager.hpp"

class GameManager {
   public:
    enum class GameState {
        SERVER,
        AUTH_SCREEN,  // Login/Register/Anonymous
        MAIN_MENU,
        LOBBY_BROWSER,  // Create lobby + list available
        IN_LOBBY,       // Inside a lobby with player list
        PLAYING,
        GAME_OVER
    };

   private:
    std::shared_ptr<Player> _player;
    std::unique_ptr<SceneManager> _scene_manager;
    Entity _timerEntity;
    Entity _bossHPEntity;
    Entity _gameStateEntity;
    Entity _boundsEntity;
    Entity _scoreTrackerEntity;
    Entity _statusDisplayEntity;
    Entity _chargeBarEntity;
    Entity _livesEntity;
    Entity _scoreDisplayEntity;
    Entity _leaderboardEntity;
    Entity _transitionTimerEntity;
    bool _gameOver = false;
    bool _victory = false;
    bool _leaderboardDisplayed = false;

    // Managers
    std::unique_ptr<AuthManager> _authManager;
    std::unique_ptr<MenuManager> _menuManager;
    std::shared_ptr<LobbyManager> _lobbyManager;

    bool _gameInitialized = false;
    Environment::GameState _previousState;
    uint32_t _localPlayerId = 0;
    bool _pendingGameStart = false;
    bool _windowHasFocus = true;

    bool _inTransition = false;
    bool _nextLevelLoaded = false;
    MasterConfig _master_config;
    EntityConfig _player_config;
    GameConfig _game_config;
    std::string _current_level_scene;
    sf::RenderWindow* _window = nullptr;
    std::shared_ptr<Environment> _env;
    std::vector<std::string> _level_files;
    int _current_level_index = 0;
    LevelConfig _current_level_config;

    // Voice chat owner
#ifdef CLIENT_BUILD
    std::unique_ptr<engine::voice::VoiceManager> _voiceManager;
#endif

    // Core methods
    void initSystems(std::shared_ptr<Environment> env);
    void initBackground(std::shared_ptr<Environment> env, const LevelConfig& config);
    void initPlayer(std::shared_ptr<Environment> env);
    void initSpawner(std::shared_ptr<Environment> env, const LevelConfig& config);
    void initScene(std::shared_ptr<Environment> env, const LevelConfig& config);
    void initUI(std::shared_ptr<Environment> env);
    void initBounds(std::shared_ptr<Environment> env);
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void setupPodControls(InputManager& inputs);
    void updateUI(std::shared_ptr<Environment> env);
    void checkGameState(std::shared_ptr<Environment> env);
    void displayGameOver(std::shared_ptr<Environment> env, bool victory);
    void displayLeaderboard(std::shared_ptr<Environment> env, bool victory);

    void startGame(std::shared_ptr<Environment> env, InputManager& inputs);

    // Server-side update loop
    void updateServer(std::shared_ptr<Environment> env, InputManager& inputs);
    std::set<uint32_t> _initializedLobbies;
    void loadLevelFiles();
    void loadNextLevel(std::shared_ptr<Environment> env);

   public:
    GameManager();
    ~GameManager() = default;

    void init(std::shared_ptr<Environment> env, InputManager& inputs);
    void update(std::shared_ptr<Environment> env, InputManager& inputs);
    void loadInputSetting(InputManager& inputs);
    Environment::GameState getCurrentState() const { return _env->getGameState(); }
    void setWindow(sf::RenderWindow* window) { _window = window; }

    // Callbacks for network events
    void onLobbyListReceived(const std::vector<LobbyInfo>& lobbies);
    void onLobbyJoined(uint32_t lobbyId, const std::string& name, const std::vector<LobbyPlayerInfo>& players,
                       uint32_t hostId);
    void onPlayerJoined(const LobbyPlayerInfo& player);
    void onPlayerLeft(uint32_t playerId);
    void onPlayerReadyChanged(uint32_t playerId, bool ready);
    void onNewHost(uint32_t hostId);
    void onGameStarted();
    void onGameEnd();
    void initGameEnd(std::shared_ptr<Environment> env);
    void onChatMessageReceived(const std::string& senderName, const std::string& message);
    void onVoicePacketReceived(const engine::voice::VoicePacket& packet);

    void setLocalPlayerId(uint32_t id) {
        _localPlayerId = id;
#ifdef CLIENT_BUILD
        if (_voiceManager) {
            _voiceManager->setLocalPlayerId(id);
        }
#endif
    }

    void onAuthSuccess();  // Called when login/register successful
    void onAuthFailed();   // Called when login failed
    void setWindowFocus(bool hasFocus) { _windowHasFocus = hasFocus; }

    // Called by server to initialize game logic via hook
    void onServerGameStart(std::shared_ptr<Environment> env, const std::vector<uint32_t>& clients, uint32_t lobbyId);

    // Called by server/client to create a player for a specific client
    std::shared_ptr<Player> createPlayerForClient(std::shared_ptr<Environment> env, uint32_t clientId, float x,
                                                  float y);
    std::shared_ptr<LobbyManager> getLobbyManager() { return _lobbyManager; }

   private:
    uint32_t _currentLobbyId = 0;  // Current lobby ID for entity tagging
};
