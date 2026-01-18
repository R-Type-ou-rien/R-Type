#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include <optional>
#include "ECS/EcsType.hpp"
#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "GameEngineBase.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "NewRenderSystem/NewRenderSystem.hpp"
#include "AnimationSystem/AnimationSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "WindowManager.hpp"
#include "PredictionSystem.hpp"
#include "LobbyState.hpp"
#include "Voice/VoiceManager.hpp"
#include "NetworkEngine/NetworkEngine.hpp"

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
   private:
    WindowManager _window_manager;
    uint32_t _serverId = 0;
    uint32_t _clientId = 0;
    std::optional<Entity> _localPlayerEntity;
    engine::core::LobbyState _lobbyState;
    std::vector<engine::core::AvailableLobby> _availableLobbies;
    std::unique_ptr<PredictionSystem> _predictionSystem;
    PhysicsSimulationCallback _physicsLogic;

   public:
    static constexpr bool IsServer = false;

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string ip = "127.0.0.1", std::string window_name = "R-Type Client");
    ~ClientGameEngine() {}
    void setPredictionLogic(PhysicsSimulationCallback logic) { _physicsLogic = logic; }

    std::optional<Entity> getLocalPlayerEntity() const {
        if (!_localPlayerEntity.has_value())
            return std::nullopt;

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

    // Lobby methods
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
    bool getReady() const { return _lobbyState.localPlayerReady; }
    bool getUnready() const { return !_lobbyState.localPlayerReady; }
    uint32_t getClientId() const { return _network ? _network->getClientId() : 0; }
    sf::RenderWindow& getWindow() { return _window_manager.getWindow(); }
    void setReadyChangedCallback(std::function<void(uint32_t, bool)> callback) { _readyChangedCallback = callback; }
    void setPlayerJoinedCallback(std::function<void(const engine::core::LobbyPlayerInfo&)> callback) {
        _playerJoinedCallback = callback;
    }
    void setPlayerLeftCallback(std::function<void(uint32_t)> callback) { _playerLeftCallback = callback; }
    void setNewHostCallback(std::function<void(uint32_t)> callback) { _newHostCallback = callback; }
    void setLobbyJoinedCallback(
        std::function<void(uint32_t, const std::string&, const std::vector<engine::core::LobbyPlayerInfo>&, uint32_t)>
            callback) {
        _lobbyJoinedCallback = callback;
    }
    void setGameStartedCallback(std::function<void()> callback) { _gameStartedCallback = callback; }

   private:
    void handleEvent();
    void processNetworkEvents();
    void applyLocalInputs(Entity playerEntity);
    void reconcile(Entity playerEntity, const transform_component_s& serverState, uint32_t serverTick);
    void processLobbyEvents(std::map<engine::core::NetworkEngine::EventType,
                                     std::vector<network::message<engine::core::NetworkEngine::EventType>>>& pending);

    std::shared_ptr<Environment> _env;

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
