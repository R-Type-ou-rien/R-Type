#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../../Entities/Player/Player.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/RType/Common/Components/config.hpp"
#include "src/RType/Common/Components/game_timer.hpp"
#include "src/Engine/Core/Scene/SceneManager.hpp"
#include "Voice/VoiceManager.hpp"

// Lobby info for browser display
struct LobbyInfo {
    uint32_t id;
    std::string name;
    int playerCount;
    int maxPlayers;
};

// Player info for in-lobby display
struct LobbyPlayerInfo {
    uint32_t id;
    std::string name;
    bool isReady;
    bool isHost;
};

class GameManager {
   public:
    enum class GameState {
        AUTH_SCREEN,  // Login/Register/Anonymous
        MAIN_MENU,
        LOBBY_BROWSER,  // Create lobby + list available
        IN_LOBBY,       // Inside a lobby with player list
        PLAYING,
        GAME_OVER
    };

   private:
    std::unique_ptr<Player> _player;
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
    bool _gameOver = false;
    bool _victory = false;
    bool _leaderboardDisplayed = false;

    // Menu entities
    Entity _menuTitleEntity = static_cast<Entity>(-1);
    Entity _playButtonEntity = static_cast<Entity>(-1);
    Entity _menuBackgroundEntity = static_cast<Entity>(-1);
    GameState _currentState = GameState::AUTH_SCREEN;  // Start at Auth Screen
    bool _gameInitialized = false;
    bool _isAuthenticated = false;
    bool _showAuthError = false;

    // Auth Screen entities
    Entity _authTitleEntity = static_cast<Entity>(-1);
    Entity _usernameLabelEntity = static_cast<Entity>(-1);
    Entity _usernameInputEntity = static_cast<Entity>(-1);
    Entity _passwordLabelEntity = static_cast<Entity>(-1);
    Entity _passwordInputEntity = static_cast<Entity>(-1);
    Entity _loginButtonEntity = static_cast<Entity>(-1);
    Entity _registerButtonEntity = static_cast<Entity>(-1);
    Entity _anonymousButtonEntity = static_cast<Entity>(-1);
    Entity _authStatusEntity = static_cast<Entity>(-1);

    // Auth Input state
    std::string _usernameText;
    std::string _passwordText;
    int _currentAuthField = 0;  // 0: None, 1: Username, 2: Password

    // Lobby browser entities
    Entity _browserTitleEntity = static_cast<Entity>(-1);
    Entity _createLobbyButtonEntity = static_cast<Entity>(-1);
    Entity _backButtonEntity = static_cast<Entity>(-1);
    Entity _inputFieldEntity = static_cast<Entity>(-1);
    Entity _inputPromptEntity = static_cast<Entity>(-1);
    std::vector<Entity> _lobbyListEntities;
    std::vector<LobbyInfo> _availableLobbies;

    // Text input state
    std::string _inputText;
    bool _isTyping = false;
    float _cursorBlinkTimer = 0.f;
    bool _cursorVisible = true;

    // In-lobby entities
    Entity _lobbyNameEntity = static_cast<Entity>(-1);
    Entity _readyButtonEntity = static_cast<Entity>(-1);
    Entity _leaveButtonEntity = static_cast<Entity>(-1);
    Entity _startButtonEntity = static_cast<Entity>(-1);
    std::vector<Entity> _playerListEntities;

    // Chat entities and state
    Entity _chatBoxEntity = static_cast<Entity>(-1);
    Entity _chatInputEntity = static_cast<Entity>(-1);
    Entity _chatPromptEntity = static_cast<Entity>(-1);
    std::vector<Entity> _chatMessageEntities;
    std::vector<std::pair<std::string, std::string>> _chatMessages;  // (sender, message)
    std::string _chatInputText;
    bool _isChatFocused = false;
    static constexpr size_t MAX_CHAT_MESSAGES = 8;

    // Lobby state
    uint32_t _currentLobbyId = 0;
    std::string _currentLobbyName;
    std::vector<LobbyPlayerInfo> _playersInLobby;
    uint32_t _hostId = 0;
    uint32_t _localPlayerId = 0;
    bool _localPlayerReady = false;
    bool _playerListDirty = false;
    bool _pendingGameStart = false;  // Set by onGameStarted, processed in update loop

    bool _mouseWasPressed = false;
    bool _enterWasPressed = false;
    bool _escapeWasPressed = false;
    bool _windowHasFocus = true;

    MasterConfig _master_config;
    EntityConfig _player_config;
    GameConfig _game_config;
    std::string _current_level_scene;
    sf::RenderWindow* _window = nullptr;

    // Network operation callbacks (set by main.cpp)
    std::function<void()> _requestLobbyListCallback;
    std::function<void(const std::string&)> _createLobbyCallback;
    std::function<void(uint32_t)> _joinLobbyCallback;
    std::function<void(bool)> _readyCallback;
    std::function<std::vector<LobbyInfo>()> _getAvailableLobbiesCallback;

    // Auth callbacks
    std::function<void(const std::string&, const std::string&)> _loginCallback;
    std::function<void(const std::string&, const std::string&)> _registerCallback;
    std::function<void()> _anonymousLoginCallback;
    std::function<void(uint32_t)> _leaveLobbyCallback;
    std::function<void(uint32_t)> _startGameCallback;
    std::function<void(const std::string&)> _sendChatCallback;
    std::function<void(const engine::voice::VoicePacket&)> _voiceSendCallback;

    // Voice chat
#ifdef CLIENT_BUILD
    std::unique_ptr<engine::voice::VoiceManager> _voiceManager;
    bool _voiceMuted = false;
    Entity _muteButtonEntity;
#endif

    // Core methods
    void initSystems(Environment& env);
    void initBackground(Environment& env, const LevelConfig& config);
    void initPlayer(Environment& env);
    void initSpawner(Environment& env, const LevelConfig& config);
    void initScene(Environment& env, const LevelConfig& config);
    void initUI(Environment& env);
    void initBounds(Environment& env);
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void setupPodControls(InputManager& inputs);
    void updateUI(Environment& env);
    void checkGameState(Environment& env);
    void displayGameOver(Environment& env, bool victory);
    void displayLeaderboard(Environment& env, bool victory);

    // Menu methods
    void initAuthScreen(Environment& env);
    void updateAuthScreen(Environment& env, InputManager& inputs);
    void cleanupAuthScreen(Environment& env);

    void initMainMenu(Environment& env);
    void updateMainMenu(Environment& env, InputManager& inputs);
    void startGame(Environment& env, InputManager& inputs);
    void cleanupMenu(Environment& env);

    // Lobby browser methods
    void initLobbyBrowser(Environment& env);
    void updateLobbyBrowser(Environment& env, InputManager& inputs);
    void cleanupLobbyBrowser(Environment& env);
    void refreshLobbyList(Environment& env);
    void handleTextInput();

    // In-lobby methods
    void initInLobby(Environment& env);
    void updateInLobby(Environment& env, InputManager& inputs);
    void cleanupInLobby(Environment& env);
    void updatePlayerListDisplay(Environment& env);
    void updateChatDisplay(Environment& env);
    void handleChatInput();

    // Utility
    bool isMouseOverButton(float btnX, float btnY, float btnWidth, float btnHeight);
    bool areAllPlayersReady() const;
    bool isLocalPlayerHost() const;

   public:
    GameManager();
    ~GameManager() = default;

    void init(Environment& env, InputManager& inputs);
    void update(Environment& env, InputManager& inputs);
    void loadInputSetting(InputManager& inputs);
    GameState getCurrentState() const { return _currentState; }
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

    // Set network callbacks
    void setRequestLobbyListCallback(std::function<void()> cb) { _requestLobbyListCallback = cb; }
    void setCreateLobbyCallback(std::function<void(const std::string&)> cb) { _createLobbyCallback = cb; }
    void setJoinLobbyCallback(std::function<void(uint32_t)> cb) { _joinLobbyCallback = cb; }
    void setReadyCallback(std::function<void(bool)> cb) { _readyCallback = cb; }

    void setGetAvailableLobbiesCallback(std::function<std::vector<LobbyInfo>()> cb) {
        _getAvailableLobbiesCallback = cb;
    }

    void setLoginCallback(std::function<void(const std::string&, const std::string&)> cb) { _loginCallback = cb; }
    void setRegisterCallback(std::function<void(const std::string&, const std::string&)> cb) { _registerCallback = cb; }
    void setAnonymousLoginCallback(std::function<void()> cb) { _anonymousLoginCallback = cb; }
    void setLeaveLobbyCallback(std::function<void(uint32_t)> cb) { _leaveLobbyCallback = cb; }
    void setStartGameCallback(std::function<void(uint32_t)> cb) { _startGameCallback = cb; }
    void setSendChatCallback(std::function<void(const std::string&)> cb) { _sendChatCallback = cb; }
    void setVoiceSendCallback(std::function<void(const engine::voice::VoicePacket&)> cb) { _voiceSendCallback = cb; }

    void onAuthSuccess();  // Called when login/register successful
    void onAuthFailed();   // Called when login failed
    void setWindowFocus(bool hasFocus) { _windowHasFocus = hasFocus; }
};
