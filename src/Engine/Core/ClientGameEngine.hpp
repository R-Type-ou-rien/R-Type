#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <optional>
#include "ECS/EcsType.hpp"
#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "GameEngineBase.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "WindowManager.hpp"
#include "LobbyState.hpp"
#include "Voice/VoiceManager.hpp"

#define SUCCESS 0
#define FAILURE -1
#define WINDOW_H 1080
#define WINDOW_W 1920

namespace engine::core {
struct AvailableLobby {
    uint32_t id;
    std::string name;
    uint32_t playerCount;
    uint32_t maxPlayers;
};
}  // namespace engine::core

class ClientGameEngine : public GameEngineBase<ClientGameEngine> {
   public:
    enum class GameScene { MAIN_MENU, LOBBY, IN_GAME };

   private:
    WindowManager _window_manager;
    uint32_t _serverId = 0;
    uint32_t _clientId = 0;
    std::optional<Entity> _localPlayerEntity;
    engine::core::LobbyState _lobbyState;
    GameScene _currentScene = GameScene::MAIN_MENU;
    std::vector<engine::core::AvailableLobby> _availableLobbies;

   public:
    static constexpr bool IsServer = false;

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {}

    std::optional<Entity> getLocalPlayerEntity() const {
        if (!_localPlayerEntity.has_value())
            return std::nullopt;

        // Convert network GUID to local entity ID
        auto it = _networkToLocalEntity.find(_localPlayerEntity.value());
        if (it != _networkToLocalEntity.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Auth methods
    void sendLogin(const std::string& username, const std::string& password);
    void sendRegister(const std::string& username, const std::string& password);
    void sendAnonymousLogin();
    void setAuthSuccessCallback(std::function<void()> cb) { _authSuccessCallback = cb; }
    void setAuthFailedCallback(std::function<void()> cb) { _authFailedCallback = cb; }

    // Lobby callbacks setters
    void setPlayerJoinedCallback(std::function<void(const engine::core::LobbyPlayerInfo&)> cb) {
        _playerJoinedCallback = cb;
    }
    void setPlayerLeftCallback(std::function<void(uint32_t)> cb) { _playerLeftCallback = cb; }
    void setNewHostCallback(std::function<void(uint32_t)> cb) { _newHostCallback = cb; }
    void setReadyChangedCallback(std::function<void(uint32_t, bool)> cb) { _readyChangedCallback = cb; }
    void setLobbyJoinedCallback(
        std::function<void(uint32_t, const std::string&, const std::vector<engine::core::LobbyPlayerInfo>&, uint32_t)>
            cb) {
        _lobbyJoinedCallback = cb;
    }
    void setGameStartedCallback(std::function<void()> cb) { _gameStartedCallback = cb; }
    void setChatMessageCallback(std::function<void(const std::string&, const std::string&)> cb) {
        _chatMessageCallback = cb;
    }
    void setFocusChangedCallback(std::function<void(bool)> cb) { _focusChangedCallback = cb; }
    void setVoicePacketCallback(std::function<void(const engine::voice::VoicePacket&)> cb) {
        _voicePacketCallback = cb;
    }

    // Lobby methods
    GameScene getCurrentScene() const { return _currentScene; }
    const engine::core::LobbyState& getLobbyState() const { return _lobbyState; }
    void sendReady();
    void sendUnready();
    void sendStartGame();
    void createLobby(const std::string& name);
    void joinLobby(uint32_t lobbyId);
    void sendLeaveLobby(uint32_t lobbyId);
    void sendChatMessage(const std::string& message);
    void sendVoicePacket(const engine::voice::VoicePacket& packet);
    void requestLobbyList();
    const std::vector<engine::core::AvailableLobby>& getAvailableLobbies() const { return _availableLobbies; }
    uint32_t getClientId() const { return _network ? _network->getClientId() : 0; }
    sf::RenderWindow& getWindow() { return _window_manager.getWindow(); }

   private:
    void handleEvent();
    void processNetworkEvents();
    void processLobbyEvents(std::map<engine::core::NetworkEngine::EventType,
                                     std::vector<network::message<engine::core::NetworkEngine::EventType>>>& pending);

    std::function<void()> _authSuccessCallback;
    std::function<void()> _authFailedCallback;

    std::function<void(const engine::core::LobbyPlayerInfo&)> _playerJoinedCallback;
    std::function<void(uint32_t)> _playerLeftCallback;
    std::function<void(uint32_t)> _newHostCallback;
    std::function<void(uint32_t, bool)> _readyChangedCallback;
    std::function<void(uint32_t, const std::string&, const std::vector<engine::core::LobbyPlayerInfo>&, uint32_t)>
        _lobbyJoinedCallback;
    std::function<void()> _gameStartedCallback;
    std::function<void(const std::string&, const std::string&)> _chatMessageCallback;
    std::function<void(bool)> _focusChangedCallback;
    std::function<void(const engine::voice::VoicePacket&)> _voicePacketCallback;
};
