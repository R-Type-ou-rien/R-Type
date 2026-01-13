#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"
#include <cmath>
#include <ctime>
#include <algorithm>
#include <thread>
#include <chrono>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

GameManager::GameManager() {
    _player_config = ConfigLoader::loadEntityConfig("src/RType/Common/content/config/player.cfg",
                                                    ConfigLoader::getRequiredPlayerFields());
    _current_level_scene = "src/RType/Common/content/config/level1.scene";

#ifdef CLIENT_BUILD
    // Initialize voice manager
    _voiceManager = std::make_unique<engine::voice::VoiceManager>();
    engine::voice::VoiceConfig voiceConfig;
    voiceConfig.sampleRate = 48000;
    voiceConfig.framesPerBuffer = 960;  // 20ms at 48kHz
    voiceConfig.channels = 1;
    _voiceManager->setConfig(voiceConfig);
    std::cout << "[VoiceManager] Created" << std::endl;
#endif
}

void GameManager::init(Environment& env, InputManager& inputs) {
    initSystems(env);
    env.loadGameResources("src/RType/Common/content/config/r-type.json");

    if (!env.isServer()) {
        initAuthScreen(env);
        _currentState = GameState::AUTH_SCREEN;
    } else {
        _currentState = GameState::LOBBY_BROWSER;
    }
}

void GameManager::startGame(Environment& env, InputManager& inputs) {
    if (_gameInitialized)
        return;

    LevelConfig level_config;
    try {
        level_config = SceneLoader::loadFromFile(_current_level_scene);
    } catch (...) {}

    initBackground(env, level_config);
    initBounds(env);
    initPlayer(env);
    initSpawner(env, level_config);
    initScene(env, level_config);
    initUI(env);

    if (!env.isServer()) {
        auto& ecs = env.getECS();
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
    _currentState = GameState::PLAYING;
}

void GameManager::update(Environment& env, InputManager& inputs) {
    switch (_currentState) {
        case GameState::AUTH_SCREEN:
            updateAuthScreen(env, inputs);
            break;
        case GameState::MAIN_MENU:
            updateMainMenu(env, inputs);
            break;
        case GameState::LOBBY_BROWSER:
            updateLobbyBrowser(env, inputs);
            break;
        case GameState::IN_LOBBY:
            updateInLobby(env, inputs);
            break;
        case GameState::PLAYING:
            updateUI(env);
            checkGameState(env);
            break;
        case GameState::GAME_OVER:
            break;
    }
}

// ============ AUTH SCREEN ============

void GameManager::initAuthScreen(Environment& env) {
    auto& ecs = env.getECS();
    _usernameText.clear();
    _passwordText.clear();
    _currentAuthField = 0;
    _showAuthError = false;

    _authTitleEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _authTitleEntity, {"AUTHENTICATION", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 60,
                           sf::Color::Cyan, 650, 100});

    // Username
    _usernameLabelEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _usernameLabelEntity, {"Username:", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 30,
                               sf::Color::White, 600, 300});

    _usernameInputEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _usernameInputEntity, {"[ Click to type ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                               30, sf::Color(150, 150, 150), 800, 300});

    // Password
    _passwordLabelEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _passwordLabelEntity, {"Password:", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 30,
                               sf::Color::White, 600, 400});

    _passwordInputEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _passwordInputEntity, {"[ Click to type ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                               30, sf::Color(150, 150, 150), 800, 400});

    // Buttons
    _loginButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _loginButtonEntity, {"[ LOGIN ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 40,
                             sf::Color::White, 650, 600});

    _registerButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _registerButtonEntity, {"[ REGISTER ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 40,
                                sf::Color::White, 900, 600});

    _anonymousButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _anonymousButtonEntity,
        {"[ ANONYMOUS LOGIN ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 30,
         sf::Color::Yellow, 750, 750});

    _authStatusEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _authStatusEntity,
        {"", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24, sf::Color::Red, 700, 500});
}

void GameManager::cleanupAuthScreen(Environment& env) {
    auto& ecs = env.getECS();
    std::vector<Entity*> entities = {&_authTitleEntity,      &_usernameLabelEntity,   &_usernameInputEntity,
                                     &_passwordLabelEntity,  &_passwordInputEntity,   &_loginButtonEntity,
                                     &_registerButtonEntity, &_anonymousButtonEntity, &_authStatusEntity};
    for (auto* e : entities) {
        if (*e != static_cast<Entity>(-1)) {
            ecs.registry.destroyEntity(*e);
            *e = static_cast<Entity>(-1);
        }
    }
}

void GameManager::onAuthSuccess() {
    _isAuthenticated = true;
    _showAuthError = false;
}

void GameManager::onAuthFailed() {
    _showAuthError = true;
}

void GameManager::updateAuthScreen(Environment& env, InputManager& inputs) {
    // Check for transition
    if (_isAuthenticated) {
        cleanupAuthScreen(env);
        initMainMenu(env);
        _currentState = GameState::MAIN_MENU;
        return;
    }

    // Ignore inputs if window doesn't have focus
    if (!_windowHasFocus) {
        _mouseWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    auto& ecs = env.getECS();

    // Button positions (simplified rects)
    const float loginX = 650, loginY = 600, loginW = 200, loginH = 50;
    const float regX = 900, regY = 600, regW = 250, regH = 50;
    const float anonX = 750, anonY = 750, anonW = 400, anonH = 40;

    // Input field areas
    const float userX = 800, userY = 300, userW = 400, userH = 40;
    const float passX = 800, passY = 400, passW = 400, passH = 40;

    if (mousePressed && !_mouseWasPressed) {
        // Field selection
        if (isMouseOverButton(userX, userY, userW, userH))
            _currentAuthField = 1;
        else if (isMouseOverButton(passX, passY, passW, passH))
            _currentAuthField = 2;
        else
            _currentAuthField = 0;

        // Buttons
        if (isMouseOverButton(loginX, loginY, loginW, loginH)) {
            if (_loginCallback && !_usernameText.empty() && !_passwordText.empty()) {
                _loginCallback(_usernameText, _passwordText);
                // Assume success for UI transition, real validation comes from server response
                // For now, wait for server response. But to test UI flow:
            }
        } else if (isMouseOverButton(regX, regY, regW, regH)) {
            if (_registerCallback && !_usernameText.empty() && !_passwordText.empty()) {
                _registerCallback(_usernameText, _passwordText);
            }
        } else if (isMouseOverButton(anonX, anonY, anonW, anonH)) {
            if (_anonymousLoginCallback) {
                _anonymousLoginCallback();
            }
        }
    }
    _mouseWasPressed = mousePressed;

    // Handle typing
    if (_currentAuthField > 0) {
        // Reuse handleTextInput logic but redirected
        _inputText = (_currentAuthField == 1) ? _usernameText : _passwordText;
        handleTextInput();  // Modifies _inputText
        if (_currentAuthField == 1)
            _usernameText = _inputText;
        else
            _passwordText = _inputText;
    }

    // Update Visuals
    if (ecs.registry.hasComponent<TextComponent>(_usernameInputEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_usernameInputEntity);
        text.text = _usernameText + (_currentAuthField == 1 ? "|" : "");
        text.color = (_currentAuthField == 1) ? sf::Color::Yellow : sf::Color::White;
        if (_usernameText.empty() && _currentAuthField != 1) {
            text.text = "[ Click to type ]";
            text.color = sf::Color(150, 150, 150);
        }
    }
    if (ecs.registry.hasComponent<TextComponent>(_passwordInputEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_passwordInputEntity);
        std::string masked(_passwordText.length(), '*');
        text.text = masked + (_currentAuthField == 2 ? "|" : "");
        text.color = (_currentAuthField == 2) ? sf::Color::Yellow : sf::Color::White;
        if (_passwordText.empty() && _currentAuthField != 2) {
            text.text = "[ Click to type ]";
            text.color = sf::Color(150, 150, 150);
        }
    }

    // Update Error Message
    if (ecs.registry.hasComponent<TextComponent>(_authStatusEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_authStatusEntity);
        if (_showAuthError) {
            text.text = "Login Failed! Try again.";
        } else {
            text.text = "";
        }
    }
}

// ============ MAIN MENU ============

void GameManager::initMainMenu(Environment& env) {
    auto& ecs = env.getECS();

    _menuTitleEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _menuTitleEntity,
        {"R-TYPE", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 100, sf::Color::Cyan, 750, 200});

    _playButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _playButtonEntity, {"[ PLAY ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 60,
                            sf::Color::White, 820, 500});
}

bool GameManager::isMouseOverButton(float btnX, float btnY, float btnWidth, float btnHeight) {
    sf::Vector2i mousePos = _window ? sf::Mouse::getPosition(*_window) : sf::Mouse::getPosition();
    float mx = static_cast<float>(mousePos.x);
    float my = static_cast<float>(mousePos.y);
    return mx >= btnX && mx <= btnX + btnWidth && my >= btnY && my <= btnY + btnHeight;
}

void GameManager::updateMainMenu(Environment& env, InputManager& inputs) {
    // Ignore inputs if window doesn't have focus
    if (!_windowHasFocus) {
        _mouseWasPressed = false;
        _enterWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    const float playBtnX = 820.0f, playBtnY = 500.0f, playBtnW = 200.0f, playBtnH = 70.0f;

    if (mousePressed && !_mouseWasPressed && isMouseOverButton(playBtnX, playBtnY, playBtnW, playBtnH)) {
        cleanupMenu(env);
        initLobbyBrowser(env);
        _currentState = GameState::LOBBY_BROWSER;
    }
    _mouseWasPressed = mousePressed;

    if (enterPressed && !_enterWasPressed) {
        cleanupMenu(env);
        initLobbyBrowser(env);
        _currentState = GameState::LOBBY_BROWSER;
    }
    _enterWasPressed = enterPressed;

    auto& ecs = env.getECS();
    if (_playButtonEntity != static_cast<Entity>(-1) && ecs.registry.hasComponent<TextComponent>(_playButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_playButtonEntity);
        float pulse = (std::sin(static_cast<float>(std::clock()) / 500.0f) + 1.0f) / 2.0f;
        text.color =
            isMouseOverButton(playBtnX, playBtnY, playBtnW, playBtnH)
                ? sf::Color::Yellow
                : sf::Color(static_cast<uint8_t>(155 + 100 * pulse), static_cast<uint8_t>(155 + 100 * pulse), 255);
    }
}

void GameManager::cleanupMenu(Environment& env) {
    auto& ecs = env.getECS();
    if (_menuTitleEntity != static_cast<Entity>(-1)) {
        ecs.registry.destroyEntity(_menuTitleEntity);
        _menuTitleEntity = static_cast<Entity>(-1);
    }
    if (_playButtonEntity != static_cast<Entity>(-1)) {
        ecs.registry.destroyEntity(_playButtonEntity);
        _playButtonEntity = static_cast<Entity>(-1);
    }
}

// ============ LOBBY BROWSER ============

void GameManager::initLobbyBrowser(Environment& env) {
    auto& ecs = env.getECS();
    _isTyping = false;
    _inputText.clear();

    // Request lobby list from server
    if (_requestLobbyListCallback) {
        _requestLobbyListCallback();
    }

    _browserTitleEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _browserTitleEntity,
        {"LOBBIES", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 48, sf::Color::Cyan, 420, 50});

    _createLobbyButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _createLobbyButtonEntity,
        {"[ + CREATE LOBBY ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28, sf::Color::White,
         380, 160});

    _inputPromptEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _inputPromptEntity,
        {"", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24, sf::Color::White, 380, 210});

    _backButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _backButtonEntity,
        {"[ BACK ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24, sf::Color::White, 50, 420});

    // Show available lobbies hint
    Entity hintEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        hintEntity, {"Available lobbies:", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 20,
                     sf::Color(150, 150, 150), 380, 300});
    _lobbyListEntities.push_back(hintEntity);

    // Get lobbies from engine callback
    if (_getAvailableLobbiesCallback) {
        _availableLobbies = _getAvailableLobbiesCallback();
    }

    // Display lobbies or "no lobbies" message
    if (_availableLobbies.empty()) {
        Entity noLobbiesEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            noLobbiesEntity,
            {"(No lobbies available - create one!)", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
             18, sf::Color(120, 120, 120), 380, 350});
        _lobbyListEntities.push_back(noLobbiesEntity);
    } else {
        float y = 350.0f;
        for (const auto& lobby : _availableLobbies) {
            Entity lobbyEntry = ecs.registry.createEntity();
            std::string text = "[ " + lobby.name + " ] (" + std::to_string(lobby.playerCount) + "/" +
                               std::to_string(lobby.maxPlayers) + ")";
            ecs.registry.addComponent<TextComponent>(
                lobbyEntry, {text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 22,
                             sf::Color::White, 380, y});
            _lobbyListEntities.push_back(lobbyEntry);
            y += 40.0f;
        }
    }
}

void GameManager::handleTextInput() {
    // Simple debouncing using static state
    uint32_t _localPlayerId = 0;
    bool _localPlayerReady = false;
    bool _playerListDirty = false;

    bool _mouseWasPressed = false;

    static int lastKey = -1;
    static bool backspaceWasPressed = false;
    static bool spaceWasPressed = false;

    int currentKey = -1;

    // Check letters A-Z
    for (int key = static_cast<int>(sf::Keyboard::Key::A); key <= static_cast<int>(sf::Keyboard::Key::Z); ++key) {
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key))) {
            currentKey = key;
            break;
        }
    }
    // Check numbers 0-9
    if (currentKey == -1) {
        for (int key = static_cast<int>(sf::Keyboard::Key::Num0); key <= static_cast<int>(sf::Keyboard::Key::Num9);
             ++key) {
            if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key))) {
                currentKey = key;
                break;
            }
        }
    }

    // Process new key press (only when different from last frame)
    if (currentKey != -1 && currentKey != lastKey && _inputText.length() < 20) {
        if (currentKey >= static_cast<int>(sf::Keyboard::Key::A) &&
            currentKey <= static_cast<int>(sf::Keyboard::Key::Z)) {
            char c = 'A' + (currentKey - static_cast<int>(sf::Keyboard::Key::A));
            if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) &&
                !sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                c = c - 'A' + 'a';
            }
            _inputText += c;
        } else if (currentKey >= static_cast<int>(sf::Keyboard::Key::Num0) &&
                   currentKey <= static_cast<int>(sf::Keyboard::Key::Num9)) {
            char c = '0' + (currentKey - static_cast<int>(sf::Keyboard::Key::Num0));
            _inputText += c;
        }
    }
    lastKey = currentKey;

    // Space with debounce
    bool spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    if (spacePressed && !spaceWasPressed && _inputText.length() < 20) {
        _inputText += ' ';
    }
    spaceWasPressed = spacePressed;

    // Backspace with debounce
    bool backspacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Backspace);
    if (backspacePressed && !backspaceWasPressed && !_inputText.empty()) {
        _inputText.pop_back();
    }
    backspaceWasPressed = backspacePressed;
}

void GameManager::updateLobbyBrowser(Environment& env, InputManager& inputs) {
    // Ignore inputs if window doesn't have focus
    if (!_windowHasFocus) {
        _mouseWasPressed = false;
        _enterWasPressed = false;
        _escapeWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    bool escapePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    auto& ecs = env.getECS();

    const float createBtnX = 380.0f, createBtnY = 160.0f, createBtnW = 300.0f, createBtnH = 40.0f;
    const float backBtnX = 50.0f, backBtnY = 420.0f, backBtnW = 120.0f, backBtnH = 35.0f;

    // Refresh lobby list UI
    refreshLobbyList(env);

    if (_isTyping) {
        // Handle keyboard text input
        handleTextInput();

        // Update input display
        if (ecs.registry.hasComponent<TextComponent>(_inputPromptEntity)) {
            auto& text = ecs.registry.getComponent<TextComponent>(_inputPromptEntity);
            _cursorBlinkTimer += 0.016f;
            if (_cursorBlinkTimer > 0.5f) {
                _cursorVisible = !_cursorVisible;
                _cursorBlinkTimer = 0.f;
            }
            text.text = "Name: " + _inputText + (_cursorVisible ? "|" : " ");
            text.color = sf::Color::Yellow;
        }

        // Confirm with Enter
        if (enterPressed && !_enterWasPressed && !_inputText.empty()) {
            // Send create lobby to server
            if (_createLobbyCallback) {
                _createLobbyCallback(_inputText);
            }
            _currentLobbyName = _inputText;
            _currentLobbyId = 999;  // Temporary ID until server confirms
            _playersInLobby.clear();
            // Player will be added by S_PLAYER_JOINED from server
            _hostId = _localPlayerId;
            cleanupLobbyBrowser(env);
            initInLobby(env);
            _currentState = GameState::IN_LOBBY;
        }
        // Cancel with Escape
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
            _isTyping = false;
            _inputText.clear();
            if (ecs.registry.hasComponent<TextComponent>(_inputPromptEntity)) {
                auto& text = ecs.registry.getComponent<TextComponent>(_inputPromptEntity);
                text.text = "";
            }
        }
    } else {
        // Button clicks
        if (mousePressed && !_mouseWasPressed) {
            if (isMouseOverButton(createBtnX, createBtnY, createBtnW, createBtnH)) {
                _isTyping = true;
                _inputText.clear();
            }
            if (isMouseOverButton(backBtnX, backBtnY, backBtnW, backBtnH)) {
                cleanupLobbyBrowser(env);
                initMainMenu(env);
                _currentState = GameState::MAIN_MENU;
            }
            // Click on lobby to join - must match display position (380, 350)
            float lobbyY = 350.0f;
            for (size_t i = 0; i < _availableLobbies.size(); i++) {
                if (isMouseOverButton(380.0f, lobbyY + i * 40.0f, 300.0f, 35.0f)) {
                    auto& lobby = _availableLobbies[i];
                    if (_joinLobbyCallback) {
                        _joinLobbyCallback(lobby.id);
                    }
                    _currentLobbyId = lobby.id;
                    _currentLobbyName = lobby.name;
                    _playersInLobby.clear();
                    // Players will be populated by S_PLAYER_JOINED from server
                    _hostId = 0;  // Will be set by S_ROOM_JOINED
                    cleanupLobbyBrowser(env);
                    initInLobby(env);
                    _currentState = GameState::IN_LOBBY;
                }
            }
        }
        if (escapePressed && !_escapeWasPressed) {
            cleanupLobbyBrowser(env);
            initMainMenu(env);
            _currentState = GameState::MAIN_MENU;
        }
    }
    _mouseWasPressed = mousePressed;
    _enterWasPressed = enterPressed;
    _escapeWasPressed = escapePressed;

    // Highlight buttons
    if (ecs.registry.hasComponent<TextComponent>(_createLobbyButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_createLobbyButtonEntity);
        text.color =
            isMouseOverButton(createBtnX, createBtnY, createBtnW, createBtnH) ? sf::Color::Yellow : sf::Color::White;
    }
    if (ecs.registry.hasComponent<TextComponent>(_backButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_backButtonEntity);
        text.color = isMouseOverButton(backBtnX, backBtnY, backBtnW, backBtnH) ? sf::Color::Yellow : sf::Color::White;
    }
}

void GameManager::cleanupLobbyBrowser(Environment& env) {
    auto& ecs = env.getECS();
    auto cleanup = [&](Entity& e) {
        if (e != static_cast<Entity>(-1)) {
            ecs.registry.destroyEntity(e);
            e = static_cast<Entity>(-1);
        }
    };
    cleanup(_browserTitleEntity);
    cleanup(_createLobbyButtonEntity);
    cleanup(_inputPromptEntity);
    cleanup(_backButtonEntity);
    for (auto& e : _lobbyListEntities)
        ecs.registry.destroyEntity(e);
    _lobbyListEntities.clear();
}

void GameManager::refreshLobbyList(Environment& env) {
    if (!_getAvailableLobbiesCallback)
        return;

    auto newLobbies = _getAvailableLobbiesCallback();

    // Check if changed (naive check: size or first element diff)
    // For simplicity, just check if size or IDs differ to avoid full deep comparison overkill every frame
    bool changed = false;
    if (newLobbies.size() != _availableLobbies.size()) {
        changed = true;
    } else {
        for (size_t i = 0; i < newLobbies.size(); ++i) {
            if (newLobbies[i].id != _availableLobbies[i].id ||
                newLobbies[i].playerCount != _availableLobbies[i].playerCount ||
                newLobbies[i].name != _availableLobbies[i].name) {
                changed = true;
                break;
            }
        }
    }

    if (!changed)
        return;

    _availableLobbies = newLobbies;
    auto& ecs = env.getECS();

    // Clear old entities
    for (auto& e : _lobbyListEntities) {
        if (e != static_cast<Entity>(-1))
            ecs.registry.destroyEntity(e);
    }
    _lobbyListEntities.clear();

    // Rebuild UI
    // Show available lobbies hint
    Entity hintEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        hintEntity, {"Available lobbies:", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 20,
                     sf::Color(150, 150, 150), 380, 300});
    _lobbyListEntities.push_back(hintEntity);

    if (_availableLobbies.empty()) {
        Entity noLobbiesEntity = ecs.registry.createEntity();
        ecs.registry.addComponent<TextComponent>(
            noLobbiesEntity,
            {"(No lobbies available - create one!)", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
             18, sf::Color(120, 120, 120), 380, 350});
        _lobbyListEntities.push_back(noLobbiesEntity);
    } else {
        float y = 350.0f;
        for (const auto& lobby : _availableLobbies) {
            Entity lobbyEntry = ecs.registry.createEntity();
            std::string text = "[ " + lobby.name + " ] (" + std::to_string(lobby.playerCount) + "/" +
                               std::to_string(lobby.maxPlayers) + ")";
            ecs.registry.addComponent<TextComponent>(
                lobbyEntry, {text, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 22,
                             sf::Color::White, 380, y});
            _lobbyListEntities.push_back(lobbyEntry);
            y += 40.0f;
        }
    }
}

// ============ IN-LOBBY SCREEN ============

void GameManager::initInLobby(Environment& env) {
    auto& ecs = env.getECS();

    _lobbyNameEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _lobbyNameEntity,
        {"Lobby: " + _currentLobbyName, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 48,
         sf::Color::Cyan, 700, 100});

    _readyButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _readyButtonEntity, {"[ READY ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 36,
                             sf::Color::White, 1400, 800});

    _leaveButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _leaveButtonEntity, {"[ LEAVE ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32,
                             sf::Color::White, 100, 950});

    _startButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _startButtonEntity, {"[ START GAME ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 40,
                             isLocalPlayerHost() ? sf::Color::White : sf::Color(100, 100, 100), 1300, 900});

#ifdef CLIENT_BUILD
    // Initialize voice mute/unmute button
    _voiceMuted = false;
    _muteButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _muteButtonEntity, {"[ MIC ON ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                            sf::Color::Green, 100, 850});
#endif

    // Initialize chat box
    _chatBoxEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _chatBoxEntity, {"--- Team Chat ---", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 20,
                         sf::Color::Cyan, 50, 550});

    _chatInputEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _chatInputEntity,
        {"> ", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 18, sf::Color::White, 50, 900});

    _chatPromptEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _chatPromptEntity, {"Click here to chat...", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf",
                            16, sf::Color(150, 150, 150), 80, 902});

    _chatMessages.clear();
    _chatInputText.clear();
    _isChatFocused = false;
    updateChatDisplay(env);

    updatePlayerListDisplay(env);

#ifdef CLIENT_BUILD
    // Start voice chat when entering lobby
    if (_voiceManager) {
        std::cout << "[GameManager] Setting voice local player ID to: " << _localPlayerId << std::endl;
        _voiceManager->setLocalPlayerId(_localPlayerId);
        _voiceManager->setSendCallback([this](const engine::voice::VoicePacket& packet) {
            if (_voiceSendCallback) {
                _voiceSendCallback(packet);
            }
        });
        if (!_voiceManager->start()) {
            std::cerr << "[GameManager] Failed to start voice chat" << std::endl;
        } else {
            std::cout << "[GameManager] Voice chat started" << std::endl;
        }
    }
#endif
}

void GameManager::updatePlayerListDisplay(Environment& env) {
    auto& ecs = env.getECS();

    // Clear old entries
    for (auto& e : _playerListEntities)
        ecs.registry.destroyEntity(e);
    _playerListEntities.clear();

    // Rebuild player list
    float y = 200.0f;
    for (const auto& player : _playersInLobby) {
        Entity e = ecs.registry.createEntity();
        std::string status = player.isReady ? " [READY]" : " [...]";
        std::string host = player.isHost ? " (HOST)" : "";
        sf::Color color = player.isReady ? sf::Color::Green : sf::Color::White;
        ecs.registry.addComponent<TextComponent>(
            e, {player.name + status + host, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32,
                color, 750, y});
        _playerListEntities.push_back(e);
        y += 60.0f;
    }
}

void GameManager::updateChatDisplay(Environment& env) {
    auto& ecs = env.getECS();

    // Clear old chat message entities
    for (auto& e : _chatMessageEntities)
        ecs.registry.destroyEntity(e);
    _chatMessageEntities.clear();

    // Display chat messages
    float y = 580.0f;
    size_t startIdx = _chatMessages.size() > MAX_CHAT_MESSAGES ? _chatMessages.size() - MAX_CHAT_MESSAGES : 0;
    for (size_t i = startIdx; i < _chatMessages.size(); ++i) {
        Entity e = ecs.registry.createEntity();
        std::string displayText = _chatMessages[i].first + ": " + _chatMessages[i].second;
        ecs.registry.addComponent<TextComponent>(
            e, {displayText, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 16, sf::Color::White,
                50, y});
        _chatMessageEntities.push_back(e);
        y += 28.0f;
    }

    // Update chat input display
    if (ecs.registry.hasComponent<TextComponent>(_chatInputEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_chatInputEntity);
        text.text = "> " + _chatInputText + (_isChatFocused && _cursorVisible ? "|" : "");
        text.color = _isChatFocused ? sf::Color::Yellow : sf::Color::White;
    }

    // Update/hide prompt
    if (ecs.registry.hasComponent<TextComponent>(_chatPromptEntity)) {
        auto& prompt = ecs.registry.getComponent<TextComponent>(_chatPromptEntity);
        prompt.text = (_isChatFocused || !_chatInputText.empty()) ? "" : "Click here to chat...";
    }
}

void GameManager::handleChatInput() {
    if (!_isChatFocused)
        return;

    // Handle text input for chat
    for (int k = static_cast<int>(sf::Keyboard::Key::A); k <= static_cast<int>(sf::Keyboard::Key::Z); ++k) {
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(k))) {
            char c = 'a' + (k - static_cast<int>(sf::Keyboard::Key::A));
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
                c = 'A' + (k - static_cast<int>(sf::Keyboard::Key::A));
            }
            if (_chatInputText.size() < 200) {
                _chatInputText += c;
            }
            // Simple debounce by sleeping
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return;
        }
    }

    // Numbers
    for (int k = static_cast<int>(sf::Keyboard::Key::Num0); k <= static_cast<int>(sf::Keyboard::Key::Num9); ++k) {
        if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(k))) {
            char c = '0' + (k - static_cast<int>(sf::Keyboard::Key::Num0));
            if (_chatInputText.size() < 200) {
                _chatInputText += c;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return;
        }
    }

    // Space
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        if (_chatInputText.size() < 200) {
            _chatInputText += ' ';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    // Backspace
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Backspace)) {
        if (!_chatInputText.empty()) {
            _chatInputText.pop_back();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        return;
    }
}

void GameManager::onChatMessageReceived(const std::string& senderName, const std::string& message) {
    _chatMessages.push_back({senderName, message});
    // Trim old messages
    while (_chatMessages.size() > MAX_CHAT_MESSAGES * 2) {
        _chatMessages.erase(_chatMessages.begin());
    }
}

bool GameManager::areAllPlayersReady() const {
    if (_playersInLobby.empty())
        return false;
    for (const auto& p : _playersInLobby) {
        if (!p.isReady)
            return false;
    }
    return true;
}

bool GameManager::isLocalPlayerHost() const {
    return _hostId == _localPlayerId || _hostId == 0;
}

void GameManager::updateInLobby(Environment& env, InputManager& inputs) {
    // Check if game start was triggered via network
    if (_pendingGameStart) {
        _pendingGameStart = false;
        std::cout << "[GameManager] Processing pending game start - initializing game scene" << std::endl;
        cleanupInLobby(env);

        // Initialize game scene but skip player creation (server handles it)
        LevelConfig level_config;
        try {
            level_config = SceneLoader::loadFromFile(_current_level_scene);
        } catch (...) {}

        initBackground(env, level_config);
        initBounds(env);
        // Skip initPlayer - server spawns player via network replication
        std::cout << "[GameManager] Multiplayer mode: Player spawning handled by network replication" << std::endl;
        initSpawner(env, level_config);
        initScene(env, level_config);
        initUI(env);

        if (!env.isServer()) {
            auto& ecs = env.getECS();
            Entity musicEntity = ecs.registry.createEntity();
            AudioSourceComponent music;
            music.sound_name = level_config.music_track.empty() ? "theme" : level_config.music_track;
            music.play_on_start = true;
            music.loop = true;
            music.destroy_entity_on_finish = false;
            ecs.registry.addComponent<AudioSourceComponent>(musicEntity, music);
            loadInputSetting(inputs);
            std::cout << "GameManager: Client mode - scene will be received from server" << std::endl;
        }

        _gameInitialized = true;
        _currentState = GameState::PLAYING;
        return;
    }

    // Ignore inputs if window doesn't have focus
    if (!_windowHasFocus) {
        _mouseWasPressed = false;
        _enterWasPressed = false;
        _escapeWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool escapePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    auto& ecs = env.getECS();

    const float readyBtnX = 1400.0f, readyBtnY = 800.0f, readyBtnW = 200.0f, readyBtnH = 50.0f;
    const float leaveBtnX = 100.0f, leaveBtnY = 950.0f, leaveBtnW = 180.0f, leaveBtnH = 45.0f;
    const float startBtnX = 1300.0f, startBtnY = 900.0f, startBtnW = 300.0f, startBtnH = 55.0f;
    const float chatAreaX = 50.0f, chatAreaY = 840.0f, chatAreaW = 400.0f, chatAreaH = 40.0f;
#ifdef CLIENT_BUILD
    const float muteBtnX = 100.0f, muteBtnY = 850.0f, muteBtnW = 200.0f, muteBtnH = 50.0f;
#endif

    if (mousePressed && !_mouseWasPressed) {
#ifdef CLIENT_BUILD
        // Voice mute button
        if (isMouseOverButton(muteBtnX, muteBtnY, muteBtnW, muteBtnH) && _voiceManager) {
            _voiceMuted = !_voiceMuted;
            _voiceManager->setMuted(_voiceMuted);
            std::cout << "[GameManager] Toggled mute: " << (_voiceMuted ? "MUTED" : "UNMUTED") << std::endl;
            _isChatFocused = false;
        } else
#endif
            // Check chat area click
            if (isMouseOverButton(chatAreaX, chatAreaY, chatAreaW, chatAreaH)) {
                _isChatFocused = true;
            } else {
                _isChatFocused = false;
            }

        // Ready button
        if (!_isChatFocused && isMouseOverButton(readyBtnX, readyBtnY, readyBtnW, readyBtnH)) {
            _localPlayerReady = !_localPlayerReady;
            if (_readyCallback) {
                _readyCallback(_localPlayerReady);
            }
            // We do NOT update local state immediately. We wait for S_READY_RETURN.
            // But we might want to update _localPlayerReady for button text?
            // Actually, button text relies on _localPlayerReady.
            // Let's keep _localPlayerReady toggle for responsiveness, but NOT update _playersInLobby manually.
            // _playersInLobby will be updated by onPlayerReadyChanged.
            // Or better: _localPlayerReady should ALSO be authoritative?
            // Let's trust the server. If we toggle, we assume it works.
            // But if packet lost, we desync.
            // For now, toggle _localPlayerReady is fine for button visual.
        }
        // Leave button
        if (isMouseOverButton(leaveBtnX, leaveBtnY, leaveBtnW, leaveBtnH)) {
            if (_leaveLobbyCallback) {
                _leaveLobbyCallback(_currentLobbyId);
            }
            cleanupInLobby(env);
            initLobbyBrowser(env);
            _currentState = GameState::LOBBY_BROWSER;
        }
        // Start button (host only, all ready)
        if (isMouseOverButton(startBtnX, startBtnY, startBtnW, startBtnH) && isLocalPlayerHost() &&
            areAllPlayersReady()) {
            // Send C_GAME_START to server - actual game start happens when S_GAME_START is received
            if (_startGameCallback) {
                _startGameCallback(_currentLobbyId);
            }
            // Do NOT call startGame() here - wait for S_GAME_START from server
        }
    }
    _mouseWasPressed = mousePressed;

    // Handle Enter key for chat
    bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    if (enterPressed && !_enterWasPressed && _isChatFocused && !_chatInputText.empty()) {
        if (_sendChatCallback) {
            _sendChatCallback(_chatInputText);
        }
        _chatInputText.clear();
        _isChatFocused = false;
    }
    _enterWasPressed = enterPressed;

    // Handle chat text input
    if (_isChatFocused) {
        handleChatInput();
    }

    // Update cursor blink for chat
    _cursorBlinkTimer += 0.016f;
    if (_cursorBlinkTimer >= 0.5f) {
        _cursorBlinkTimer = 0.f;
        _cursorVisible = !_cursorVisible;
    }

    // Update chat display
    updateChatDisplay(env);

    if (escapePressed && !_escapeWasPressed) {
        if (_isChatFocused) {
            _isChatFocused = false;
            _chatInputText.clear();
        } else {
            if (_leaveLobbyCallback) {
                _leaveLobbyCallback(_currentLobbyId);
            }
            cleanupInLobby(env);
            initLobbyBrowser(env);
            _currentState = GameState::LOBBY_BROWSER;
        }
    }
    _escapeWasPressed = escapePressed;

    if (_playerListDirty) {
        updatePlayerListDisplay(env);
        _playerListDirty = false;
    }

    // Update button visuals
    if (ecs.registry.hasComponent<TextComponent>(_readyButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_readyButtonEntity);
        text.text = _localPlayerReady ? "[ UNREADY ]" : "[ READY ]";
        text.color = isMouseOverButton(readyBtnX, readyBtnY, readyBtnW, readyBtnH)
                         ? sf::Color::Yellow
                         : (_localPlayerReady ? sf::Color::Green : sf::Color::White);
    }
    if (ecs.registry.hasComponent<TextComponent>(_leaveButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_leaveButtonEntity);
        text.color =
            isMouseOverButton(leaveBtnX, leaveBtnY, leaveBtnW, leaveBtnH) ? sf::Color::Yellow : sf::Color::White;
    }

#ifdef CLIENT_BUILD
    if (ecs.registry.hasComponent<TextComponent>(_muteButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_muteButtonEntity);
        text.text = _voiceMuted ? "[ MIC OFF ]" : "[ MIC ON ]";
        if (_voiceMuted) {
            text.color =
                isMouseOverButton(muteBtnX, muteBtnY, muteBtnW, muteBtnH) ? sf::Color(255, 100, 100) : sf::Color::Red;
        } else {
            text.color =
                isMouseOverButton(muteBtnX, muteBtnY, muteBtnW, muteBtnH) ? sf::Color::Green : sf::Color::White;
        }
    }
#endif

    if (ecs.registry.hasComponent<TextComponent>(_startButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_startButtonEntity);
        bool canStart = isLocalPlayerHost() && areAllPlayersReady();
        if (!isLocalPlayerHost()) {
            text.color = sf::Color(100, 100, 100);
        } else if (!areAllPlayersReady()) {
            text.color = sf::Color(150, 100, 100);
        } else {
            text.color =
                isMouseOverButton(startBtnX, startBtnY, startBtnW, startBtnH) ? sf::Color::Yellow : sf::Color::Green;
        }
    }
}

void GameManager::cleanupInLobby(Environment& env) {
    auto& ecs = env.getECS();

#ifdef CLIENT_BUILD
    // Stop voice chat when leaving lobby
    if (_voiceManager) {
        _voiceManager->stop();
        std::cout << "[GameManager] Voice chat stopped" << std::endl;
    }
#endif

    auto cleanup = [&](Entity& e) {
        if (e != static_cast<Entity>(-1)) {
            ecs.registry.destroyEntity(e);
            e = static_cast<Entity>(-1);
        }
    };
    cleanup(_lobbyNameEntity);
    cleanup(_readyButtonEntity);
    cleanup(_leaveButtonEntity);
    cleanup(_startButtonEntity);
    cleanup(_chatBoxEntity);
    cleanup(_chatInputEntity);
    cleanup(_chatPromptEntity);
#ifdef CLIENT_BUILD
    cleanup(_muteButtonEntity);
#endif
    for (auto& e : _playerListEntities)
        ecs.registry.destroyEntity(e);
    _playerListEntities.clear();
    for (auto& e : _chatMessageEntities)
        ecs.registry.destroyEntity(e);
    _chatMessageEntities.clear();
    _chatMessages.clear();
    _chatInputText.clear();
    _isChatFocused = false;
    _localPlayerReady = false;
}

// ============ NETWORK CALLBACKS ============

void GameManager::onLobbyListReceived(const std::vector<LobbyInfo>& lobbies) {
    _availableLobbies = lobbies;
}

void GameManager::onLobbyJoined(uint32_t lobbyId, const std::string& name, const std::vector<LobbyPlayerInfo>& players,
                                uint32_t hostId) {
    _currentLobbyId = lobbyId;
    _currentLobbyName = name;
    _playersInLobby = players;
    _hostId = hostId;
    _playerListDirty = true;
}

void GameManager::onPlayerJoined(const LobbyPlayerInfo& player) {
    _playersInLobby.push_back(player);
    _playerListDirty = true;
}

void GameManager::onPlayerLeft(uint32_t playerId) {
    _playersInLobby.erase(std::remove_if(_playersInLobby.begin(), _playersInLobby.end(),
                                         [playerId](const LobbyPlayerInfo& p) { return p.id == playerId; }),
                          _playersInLobby.end());
    _playerListDirty = true;
}

void GameManager::onPlayerReadyChanged(uint32_t playerId, bool ready) {
    for (auto& p : _playersInLobby) {
        if (p.id == playerId) {
            p.isReady = ready;
            break;
        }
    }
    _playerListDirty = true;
}

void GameManager::onNewHost(uint32_t hostId) {
    _hostId = hostId;
    for (auto& p : _playersInLobby) {
        p.isHost = (p.id == hostId);
    }
    _playerListDirty = true;
}

void GameManager::onGameStarted() {
    std::cout << "[GameManager] onGameStarted called, setting pending game start flag" << std::endl;
    // Set flag to be processed in update loop where we have access to Environment
    _pendingGameStart = true;
}

void GameManager::onVoicePacketReceived(const engine::voice::VoicePacket& packet) {
#ifdef CLIENT_BUILD
    if (_voiceManager) {
        static uint32_t packetsForwarded = 0;
        packetsForwarded++;
        if (packetsForwarded % 50 == 1) {
            std::cout << "[GameManager] Forwarding voice packet " << packetsForwarded << " from sender "
                      << packet.senderId << " to VoiceManager" << std::endl;
        }
        _voiceManager->receivePacket(packet);
    }
#endif
}
