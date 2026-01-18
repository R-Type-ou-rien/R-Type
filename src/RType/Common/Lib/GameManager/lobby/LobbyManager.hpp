#pragma once

#include <memory>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ECS.hpp"
#include "GameEngineBase.hpp"

// Forward declaration
namespace engine {
namespace voice {
class VoiceManager;
}
}  // namespace engine
struct VoiceManager;  // If using undefined type pointer

struct LobbyInfo {
    uint32_t id;
    std::string name;
    uint32_t playerCount;
    uint32_t maxPlayers;
};

struct LobbyPlayerInfo {
    uint32_t id;
    std::string name;
    bool isReady;
    bool isHost;
};

class LobbyManager {
   public:
    LobbyManager() = default;
    ~LobbyManager() = default;

    // Browser
    void initBrowser(std::shared_ptr<Environment> env);
    void updateBrowser(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus,
                       uint32_t localPlayerId);
    void cleanupBrowser(std::shared_ptr<Environment> env);
    void refreshLobbyList(std::shared_ptr<Environment> env);

    // In-Lobby
    void initLobby(std::shared_ptr<Environment> env, uint32_t lobbyId, const std::string& lobbyName,
                   uint32_t localPlayerId);
    void updateLobby(std::shared_ptr<Environment> env, sf::RenderWindow* window, bool hasFocus, uint32_t localPlayerId);
    void cleanupLobby(std::shared_ptr<Environment> env);

    // Setters/Getters
    void setAvailableLobbies(const std::vector<LobbyInfo>& lobbies) { _availableLobbies = lobbies; }
    void setPlayersInLobby(const std::vector<LobbyPlayerInfo>& players) {
        _playersInLobby = players;
        _playerListDirty = true;
    }
    void addPlayer(const LobbyPlayerInfo& player) {
        for (auto& p : _playersInLobby) {
            if (p.id == player.id) {
                p = player;
                _playerListDirty = true;
                return;
            }
        }
        _playersInLobby.push_back(player);
        _playerListDirty = true;
    }
    std::vector<LobbyPlayerInfo>& getPlayersInLobby() { return _playersInLobby; }
    void setHostId(uint32_t hostId) {
        _hostId = hostId;
        _playerListDirty = true;
    }
    void setVoiceManager(void* vm) { _voiceManager = vm; }  // void* to avoid complex includes, or forward declare

    // State Access
    uint32_t getCurrentLobbyId() const { return _currentLobbyId; }
    std::string getCurrentLobbyName() const { return _currentLobbyName; }
    void setLocalPlayerReady(bool ready) { _localPlayerReady = ready; }
    bool isLocalPlayerReady() const { return _localPlayerReady; }
    void setPlayerReady(uint32_t id, bool ready);

    void onChatMessageReceived(const std::string& sender, const std::string& msg);

   private:
    bool isMouseOverButton(sf::RenderWindow* window, float btnX, float btnY, float btnWidth, float btnHeight);
    void handleTextInput(std::shared_ptr<Environment> env);
    void handleChatInput(std::shared_ptr<Environment> env);
    void updatePlayerListDisplay(std::shared_ptr<Environment> env);
    void updateChatDisplay(std::shared_ptr<Environment> env);
    bool areAllPlayersReady() const;
    bool isLocalPlayerHost(uint32_t localId) const;

    // Browser Entities
    Entity _browserTitleEntity = static_cast<Entity>(-1);
    Entity _createLobbyButtonEntity = static_cast<Entity>(-1);
    Entity _backButtonEntity = static_cast<Entity>(-1);
    Entity _inputPromptEntity = static_cast<Entity>(-1);
    std::vector<Entity> _lobbyListEntities;

    // In-Lobby Entities
    Entity _lobbyNameEntity = static_cast<Entity>(-1);
    Entity _readyButtonEntity = static_cast<Entity>(-1);
    Entity _leaveButtonEntity = static_cast<Entity>(-1);
    Entity _startButtonEntity = static_cast<Entity>(-1);
    Entity _muteButtonEntity = static_cast<Entity>(-1);
    Entity _volUpButtonEntity = static_cast<Entity>(-1);
    Entity _volDownButtonEntity = static_cast<Entity>(-1);
    Entity _volDisplayEntity = static_cast<Entity>(-1);
    std::vector<Entity> _playerListEntities;

    // Chat entities
    Entity _chatBoxEntity = static_cast<Entity>(-1);
    Entity _chatInputEntity = static_cast<Entity>(-1);
    Entity _chatPromptEntity = static_cast<Entity>(-1);
    std::vector<Entity> _chatMessageEntities;

    // State
    std::vector<LobbyInfo> _availableLobbies;
    std::vector<LobbyPlayerInfo> _playersInLobby;
    std::vector<std::pair<std::string, std::string>> _chatMessages;
    uint32_t _currentLobbyId = 0;
    std::string _currentLobbyName;
    uint32_t _hostId = 0;

    // Input state
    std::string _inputText;
    std::string _chatInputText;
    bool _isTyping = false;
    bool _isChatFocused = false;
    bool _cursorVisible = true;
    float _cursorBlinkTimer = 0.f;
    bool _localPlayerReady = false;
    bool _voiceMuted = false;
    float _currentInputVolume = 0.35f;
    bool _playerListDirty = false;

    bool _mouseWasPressed = false;
    bool _enterWasPressed = false;
    bool _escapeWasPressed = false;
    static constexpr size_t MAX_CHAT_MESSAGES = 8;

    void* _voiceManager = nullptr;
};
