#include "LobbyManager.hpp"
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include "../../../../../Engine/Lib/Components/StandardComponents.hpp"
#include "Voice/VoiceManager.hpp"
#include "src/Engine/Core/ClientGameEngine.hpp"

using namespace engine::voice;

bool LobbyManager::isMouseOverButton(sf::RenderWindow* window, float btnX, float btnY, float btnWidth,
                                     float btnHeight) {
    sf::Vector2i mousePos = window ? sf::Mouse::getPosition(*window) : sf::Mouse::getPosition();
    float mx = static_cast<float>(mousePos.x);
    float my = static_cast<float>(mousePos.y);
    return mx >= btnX && mx <= btnX + btnWidth && my >= btnY && my <= btnY + btnHeight;
}

void LobbyManager::initBrowser(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();
    cleanupBrowser(env);

    _isTyping = false;
    _inputText.clear();

    if (env->hasFunction("requestLobbyList")) {
        auto func = env->getFunction<std::function<void()>>("requestLobbyList");
        func();
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

void LobbyManager::cleanupBrowser(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();
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
    for (auto& e : _lobbyListEntities) {
        if (e != static_cast<Entity>(-1))
            ecs.registry.destroyEntity(e);
    }
    _lobbyListEntities.clear();
}

void LobbyManager::handleTextInput(std::shared_ptr<Environment> env) {
    while (env->hasTextInput()) {
        uint32_t unicode = env->popTextInput();
        if (unicode == 8 && !_inputText.empty()) {  // Backspace
            _inputText.pop_back();
        } else if (unicode < 128 && unicode > 31 && _inputText.length() < 20) {
            _inputText += static_cast<char>(unicode);
        }
    }
}

void LobbyManager::updateBrowser(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus,
                                 uint32_t localPlayerId) {
    if (!hasFocus) {
        _mouseWasPressed = false;
        _enterWasPressed = false;
        _escapeWasPressed = false;
        return;
    }

    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    bool escapePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    auto& ecs = env->getECS();

    const float createBtnX = 380.0f, createBtnY = 160.0f, createBtnW = 300.0f, createBtnH = 40.0f;
    const float backBtnX = 50.0f, backBtnY = 420.0f, backBtnW = 120.0f, backBtnH = 35.0f;

    refreshLobbyList(env);

    if (_isTyping) {
        handleTextInput(env);

        if (ecs.registry.hasComponent<TextComponent>(_inputPromptEntity)) {
            auto& text = ecs.registry.getComponent<TextComponent>(_inputPromptEntity);
            _cursorBlinkTimer += 0.016f;  // Approx
            if (_cursorBlinkTimer > 0.5f) {
                _cursorVisible = !_cursorVisible;
                _cursorBlinkTimer = 0.f;
            }
            text.text = "Name: " + _inputText + (_cursorVisible ? "|" : " ");
            text.color = sf::Color::Yellow;
        }

        if (enterPressed && !_enterWasPressed && !_inputText.empty()) {
            if (env->hasFunction("createLobby")) {
                auto func = env->getFunction<std::function<void(const std::string&)>>("createLobby");
                func(_inputText);
            }
            _currentLobbyName = _inputText;
            _currentLobbyId = 999;
            _playersInLobby.clear();
            _hostId = localPlayerId;
            env->setGameState(Environment::GameState::LOBBY);
        }
        if (escapePressed) {
            _isTyping = false;
            _inputText.clear();
            if (ecs.registry.hasComponent<TextComponent>(_inputPromptEntity)) {
                auto& text = ecs.registry.getComponent<TextComponent>(_inputPromptEntity);
                text.text = "";
            }
            return;
        }
    } else {
        if (mousePressed && !_mouseWasPressed) {
            if (isMouseOverButton(window, createBtnX, createBtnY, createBtnW, createBtnH)) {
                _isTyping = true;
                _inputText.clear();
            }
            if (isMouseOverButton(window, backBtnX, backBtnY, backBtnW, backBtnH)) {
                env->setGameState(Environment::GameState::MAIN_MENU);
            }
            float lobbyY = 350.0f;
            for (size_t i = 0; i < _availableLobbies.size(); i++) {
                if (isMouseOverButton(window, 380.0f, lobbyY + i * 40.0f, 300.0f, 35.0f)) {
                    auto& lobby = _availableLobbies[i];
                    if (env->hasFunction("joinLobby")) {
                        auto func = env->getFunction<std::function<void(uint32_t)>>("joinLobby");
                        func(lobby.id);
                    }
                    _currentLobbyId = lobby.id;
                    _currentLobbyName = lobby.name;
                    _playersInLobby.clear();
                    _hostId = 0;
                    env->setGameState(Environment::GameState::LOBBY);
                }
            }
        }
        if (escapePressed && !_escapeWasPressed) {
            env->setGameState(Environment::GameState::MAIN_MENU);
        }
    }
    _mouseWasPressed = mousePressed;
    _enterWasPressed = enterPressed;
    _escapeWasPressed = escapePressed;

    if (ecs.registry.hasComponent<TextComponent>(_createLobbyButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_createLobbyButtonEntity);
        text.color = isMouseOverButton(window, createBtnX, createBtnY, createBtnW, createBtnH) ? sf::Color::Yellow
                                                                                               : sf::Color::White;
    }
    if (ecs.registry.hasComponent<TextComponent>(_backButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_backButtonEntity);
        text.color =
            isMouseOverButton(window, backBtnX, backBtnY, backBtnW, backBtnH) ? sf::Color::Yellow : sf::Color::White;
    }
}

void LobbyManager::refreshLobbyList(std::shared_ptr<Environment> env) {
    std::vector<LobbyInfo> newLobbies;
    if (env->hasFunction("getAvailableLobbies")) {
        auto getLobbies =
            env->getFunction<std::function<std::vector<engine::core::AvailableLobby>()>>("getAvailableLobbies");
        auto engineLobbies = getLobbies();
        for (const auto& lobby : engineLobbies) {
            newLobbies.push_back({lobby.id, lobby.name, lobby.playerCount, lobby.maxPlayers});
        }
    }

    bool changed = false;
    if (newLobbies.empty() && _availableLobbies.empty())
        return;

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
    auto& ecs = env->getECS();

    for (auto& e : _lobbyListEntities) {
        if (e != static_cast<Entity>(-1))
            ecs.registry.destroyEntity(e);
    }
    _lobbyListEntities.clear();

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

// ================== In-Lobby ==================

void LobbyManager::initLobby(std::shared_ptr<Environment> env, uint32_t lobbyId, const std::string& lobbyName,
                             uint32_t localPlayerId) {
    auto& ecs = env->getECS();
    cleanupLobby(env);

    _currentLobbyId = lobbyId;
    _currentLobbyName = lobbyName;

#ifdef CLIENT_BUILD
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
        _startButtonEntity,
        {"[ START GAME ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 40,
         isLocalPlayerHost(localPlayerId) ? sf::Color::White : sf::Color(100, 100, 100), 1300, 900});

    _voiceMuted = false;
    _muteButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _muteButtonEntity, {"[ MIC ON ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 28,
                            sf::Color::Green, 100, 500});

    _volDownButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _volDownButtonEntity,
        {"[ - ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32, sf::Color::White, 350, 500});

    _volDisplayEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _volDisplayEntity, {"Vol: 35%", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 24,
                            sf::Color::White, 420, 506});

    _volUpButtonEntity = ecs.registry.createEntity();
    ecs.registry.addComponent<TextComponent>(
        _volUpButtonEntity,
        {"[ + ]", "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32, sf::Color::White, 550, 500});

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

    if (_voiceManager) {
        engine::voice::VoiceManager* vm = static_cast<engine::voice::VoiceManager*>(_voiceManager);
        vm->setLocalPlayerId(localPlayerId);
        vm->setSendCallback([env](const engine::voice::VoicePacket& packet) {
            if (env->hasFunction("sendVoicePacket")) {
                auto func = env->getFunction<std::function<void(const engine::voice::VoicePacket&)>>("sendVoicePacket");
                func(packet);
            }
        });
        if (!vm->start()) {
            std::cerr << "[LobbyManager] Failed to start voice chat" << std::endl;
        } else {
            std::cout << "[LobbyManager] Voice chat started" << std::endl;
        }
    }
#endif

    // Register callback for receiving chat messages
    env->addFunction("receiveChatMessage",
                     std::function<void(const std::string&, const std::string&)>(
                         [this](const std::string& s, const std::string& m) { this->onChatMessageReceived(s, m); }));

#ifdef CLIENT_BUILD
    // Register callback for receiving voice packets
    env->addFunction(
        "receiveVoicePacket",
        std::function<void(const engine::voice::VoicePacket&)>([this](const engine::voice::VoicePacket& p) {
            if (_voiceManager) {
                static_cast<engine::voice::VoiceManager*>(_voiceManager)->receivePacket(p);
            }
        }));
#endif
}

void LobbyManager::cleanupLobby(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();

#ifdef CLIENT_BUILD
    if (_voiceManager) {
        engine::voice::VoiceManager* vm = static_cast<engine::voice::VoiceManager*>(_voiceManager);
        vm->stop();
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
    cleanup(_muteButtonEntity);
    cleanup(_volUpButtonEntity);
    cleanup(_volDownButtonEntity);
    cleanup(_volDisplayEntity);

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

void LobbyManager::updateLobby(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus,
                               uint32_t localPlayerId) {
    if (!hasFocus) {
        _mouseWasPressed = false;
        _enterWasPressed = false;
        _escapeWasPressed = false;
        return;
    }
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool escapePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    bool enterPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    auto& ecs = env->getECS();

    const float readyBtnX = 1400.0f, readyBtnY = 800.0f, readyBtnW = 200.0f, readyBtnH = 50.0f;
    const float leaveBtnX = 100.0f, leaveBtnY = 950.0f, leaveBtnW = 180.0f, leaveBtnH = 45.0f;
    const float startBtnX = 1300.0f, startBtnY = 900.0f, startBtnW = 300.0f, startBtnH = 55.0f;
    const float chatAreaX = 50.0f, chatAreaY = 570.0f, chatAreaW = 400.0f, chatAreaH = 316.0f;
#ifdef CLIENT_BUILD
    const float muteBtnX = 100.0f, muteBtnY = 500.0f, muteBtnW = 200.0f, muteBtnH = 50.0f;
    const float volDownBtnX = 350.0f, volDownBtnY = 500.0f, volDownBtnW = 60.0f, volDownBtnH = 50.0f;
    const float volUpBtnX = 550.0f, volUpBtnY = 500.0f, volUpBtnW = 60.0f, volUpBtnH = 50.0f;
#endif

    if (mousePressed && !_mouseWasPressed) {
#ifdef CLIENT_BUILD
        if (_voiceManager) {
            engine::voice::VoiceManager* vm = static_cast<engine::voice::VoiceManager*>(_voiceManager);

            if (isMouseOverButton(window, muteBtnX, muteBtnY, muteBtnW, muteBtnH)) {
                _voiceMuted = !_voiceMuted;
                vm->setMuted(_voiceMuted);
                _isChatFocused = false;
            } else if (isMouseOverButton(window, volDownBtnX, volDownBtnY, volDownBtnW, volDownBtnH)) {
                _currentInputVolume -= 0.10f;
                if (_currentInputVolume < 0.0f)
                    _currentInputVolume = 0.0f;
                vm->setInputVolume(_currentInputVolume);
                _isChatFocused = false;
            } else if (isMouseOverButton(window, volUpBtnX, volUpBtnY, volUpBtnW, volUpBtnH)) {
                _currentInputVolume += 0.10f;
                if (_currentInputVolume > 1.0f)
                    _currentInputVolume = 1.0f;
                vm->setInputVolume(_currentInputVolume);
                _isChatFocused = false;
            } else if (isMouseOverButton(window, chatAreaX, chatAreaY, chatAreaW, chatAreaH)) {
                _isChatFocused = true;
            } else {
                _isChatFocused = false;
            }
        }
#endif

        if (!_isChatFocused && isMouseOverButton(window, readyBtnX, readyBtnY, readyBtnW, readyBtnH)) {
            _localPlayerReady = !_localPlayerReady;
            if (env->hasFunction("sendReady")) {
                auto func = env->getFunction<std::function<void(bool)>>("sendReady");
                func(_localPlayerReady);
            }
        }
        if (isMouseOverButton(window, leaveBtnX, leaveBtnY, leaveBtnW, leaveBtnH)) {
            if (env->hasFunction("leaveLobby")) {
                auto func = env->getFunction<std::function<void(uint32_t)>>("leaveLobby");
                func(_currentLobbyId);
            }
            env->setGameState(Environment::GameState::LOBBY_LIST);
        }
        if (isMouseOverButton(window, startBtnX, startBtnY, startBtnW, startBtnH) && isLocalPlayerHost(localPlayerId) &&
            areAllPlayersReady()) {
            if (env->hasFunction("startGame")) {
                auto func = env->getFunction<std::function<void(uint32_t)>>("startGame");
                func(_currentLobbyId);
            }
        }
    }

    handleChatInput(env);

    if (enterPressed && !_enterWasPressed && _isChatFocused && !_chatInputText.empty()) {
        if (env->hasFunction("sendChatMessage")) {
            auto func = env->getFunction<std::function<void(const std::string&)>>("sendChatMessage");
            func(_chatInputText);
        }
        _chatInputText.clear();
    }

    _cursorBlinkTimer += 0.016f;
    if (_cursorBlinkTimer >= 0.5f) {
        _cursorBlinkTimer = 0.f;
        _cursorVisible = !_cursorVisible;
    }

    updateChatDisplay(env);

    if (escapePressed && !_escapeWasPressed) {
        if (_isChatFocused) {
            _isChatFocused = false;
            _chatInputText.clear();
        } else {
            if (env->hasFunction("leaveLobby")) {
                auto func = env->getFunction<std::function<void(uint32_t)>>("leaveLobby");
                func(_currentLobbyId);
            }
            env->setGameState(Environment::GameState::LOBBY_LIST);
        }
    }

    _enterWasPressed = enterPressed;
    _escapeWasPressed = escapePressed;
    _mouseWasPressed = mousePressed;

    if (_playerListDirty) {
        updatePlayerListDisplay(env);
        _playerListDirty = false;
    }

    // Update Button Visuals
    if (ecs.registry.hasComponent<TextComponent>(_readyButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_readyButtonEntity);
        text.text = _localPlayerReady ? "[ UNREADY ]" : "[ READY ]";
        text.color = isMouseOverButton(window, readyBtnX, readyBtnY, readyBtnW, readyBtnH)
                         ? sf::Color::Yellow
                         : (_localPlayerReady ? sf::Color::Green : sf::Color::White);
    }
    if (ecs.registry.hasComponent<TextComponent>(_leaveButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_leaveButtonEntity);
        text.color = isMouseOverButton(window, leaveBtnX, leaveBtnY, leaveBtnW, leaveBtnH) ? sf::Color::Yellow
                                                                                           : sf::Color::White;
    }
#ifdef CLIENT_BUILD
    if (ecs.registry.hasComponent<TextComponent>(_muteButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_muteButtonEntity);
        text.text = _voiceMuted ? "[ MIC OFF ]" : "[ MIC ON ]";
        if (_voiceMuted) {
            text.color = isMouseOverButton(window, muteBtnX, muteBtnY, muteBtnW, muteBtnH) ? sf::Color(255, 100, 100)
                                                                                           : sf::Color::Red;
        } else {
            text.color =
                isMouseOverButton(window, muteBtnX, muteBtnY, muteBtnW, muteBtnH) ? sf::Color::Green : sf::Color::White;
        }
    }

    // Volume Controls Visuals

    if (ecs.registry.hasComponent<TextComponent>(_volDownButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_volDownButtonEntity);
        text.color = isMouseOverButton(window, volDownBtnX, volDownBtnY, volDownBtnW, volDownBtnH) ? sf::Color::Yellow
                                                                                                   : sf::Color::White;
    }
    if (ecs.registry.hasComponent<TextComponent>(_volUpButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_volUpButtonEntity);
        text.color = isMouseOverButton(window, volUpBtnX, volUpBtnY, volUpBtnW, volUpBtnH) ? sf::Color::Yellow
                                                                                           : sf::Color::White;
    }
    if (ecs.registry.hasComponent<TextComponent>(_volDisplayEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_volDisplayEntity);
        int volPercent = static_cast<int>(_currentInputVolume * 100.0f + 0.5f);  // Rounding
        text.text = "Vol: " + std::to_string(volPercent) + "%";
    }
#endif
    if (ecs.registry.hasComponent<TextComponent>(_startButtonEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_startButtonEntity);
        if (!isLocalPlayerHost(localPlayerId)) {
            text.color = sf::Color(100, 100, 100);
        } else if (!areAllPlayersReady()) {
            text.color = sf::Color(150, 100, 100);
        } else {
            text.color = isMouseOverButton(window, startBtnX, startBtnY, startBtnW, startBtnH) ? sf::Color::Yellow
                                                                                               : sf::Color::Green;
        }
    }
}

void LobbyManager::setPlayerReady(uint32_t id, bool ready) {
    for (auto& p : _playersInLobby) {
        if (p.id == id) {
            std::cout << "[LobbyManager] Setting player " << id << " ready=" << ready << ". Old status=" << p.isReady
                      << std::endl;
            p.isReady = ready;
            _playerListDirty = true;
            return;
        }
    }
    std::cout << "[LobbyManager] Failed to set ready for player " << id << " (not found)" << std::endl;
}

void LobbyManager::updatePlayerListDisplay(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();

    for (auto& e : _playerListEntities)
        ecs.registry.destroyEntity(e);
    _playerListEntities.clear();

    float y = 200.0f;
    for (const auto& player : _playersInLobby) {
        Entity e = ecs.registry.createEntity();
        std::string status = player.isReady ? " [READY]" : " [...]";
        std::string host = player.isHost ? " (HOST)" : "";
        sf::Color color = player.isReady ? sf::Color::Green : sf::Color::White;
        if (color == sf::Color::Black)
            color = sf::Color::White;

        ecs.registry.addComponent<TextComponent>(
            e, {player.name + status + host, "src/RType/Common/content/open_dyslexic/OpenDyslexic-Regular.otf", 32,
                color, 750, y});
        _playerListEntities.push_back(e);
        y += 60.0f;
    }
}

void LobbyManager::updateChatDisplay(std::shared_ptr<Environment> env) {
    auto& ecs = env->getECS();

    for (auto& e : _chatMessageEntities)
        ecs.registry.destroyEntity(e);
    _chatMessageEntities.clear();

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

    if (ecs.registry.hasComponent<TextComponent>(_chatInputEntity)) {
        auto& text = ecs.registry.getComponent<TextComponent>(_chatInputEntity);
        text.text = "> " + _chatInputText + (_isChatFocused && _cursorVisible ? "|" : "");
        text.color = _isChatFocused ? sf::Color::Yellow : sf::Color::White;
    }

    if (ecs.registry.hasComponent<TextComponent>(_chatPromptEntity)) {
        auto& prompt = ecs.registry.getComponent<TextComponent>(_chatPromptEntity);
        prompt.text = (_isChatFocused || !_chatInputText.empty()) ? "" : "Click here to chat...";
    }
}

void LobbyManager::handleChatInput(std::shared_ptr<Environment> env) {
    if (!_isChatFocused)
        return;

    while (env->hasTextInput()) {
        uint32_t unicode = env->popTextInput();
        if (unicode == 8 && !_chatInputText.empty()) {  // Backspace
            _chatInputText.pop_back();
        } else if (unicode < 128 && unicode > 31 && _chatInputText.length() < 200) {
            _chatInputText += static_cast<char>(unicode);
        }
    }
}

void LobbyManager::onChatMessageReceived(const std::string& sender, const std::string& msg) {
    _chatMessages.push_back({sender, msg});
    while (_chatMessages.size() > MAX_CHAT_MESSAGES * 2) {
        _chatMessages.erase(_chatMessages.begin());
    }
}

bool LobbyManager::areAllPlayersReady() const {
    if (_playersInLobby.empty())
        return false;
    for (const auto& p : _playersInLobby) {
        if (!p.isReady)
            return false;
    }
    return true;
}

bool LobbyManager::isLocalPlayerHost(uint32_t localId) const {
    return _hostId == localId || _hostId == 0;
}
