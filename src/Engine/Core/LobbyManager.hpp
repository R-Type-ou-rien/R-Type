#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <optional>

namespace engine {
namespace core {

struct ClientInfo {
    uint32_t id;
    std::string name;
    bool isReady = false;
};

class Lobby {
      public:
    enum class State {
        WAITING,
        IN_GAME,
    };

    explicit Lobby(uint32_t id, std::string name, uint32_t maxPlayers)
        : _id(id), _name(std::move(name)), _maxPlayers(maxPlayers), _state(State::WAITING) {}

    bool addClient(const ClientInfo& client);
    bool removeClient(uint32_t clientId);
    void setPlayerReady(uint32_t clientId, bool ready);
    bool areAllPlayersReady() const;

    uint32_t getId() const { return _id; }
    const std::string& getName() const { return _name; }
    State getState() const { return _state; }
    void setState(State state) { _state = state; }
    size_t getPlayerCount() const { return _clients.size(); }
    uint32_t getMaxPlayers() const { return _maxPlayers; }
    const std::vector<ClientInfo>& getClients() const { return _clients; }
    bool isFull() const { return _clients.size() >= _maxPlayers; }

    uint32_t getHostId() const { return _hostId; }
    void setHostId(uint32_t hostId) { _hostId = hostId; }
    bool isHost(uint32_t clientId) const { return _hostId == clientId; }

   private:
    uint32_t _id;
    std::string _name;
    uint32_t _maxPlayers;
    State _state;
    std::vector<ClientInfo> _clients;
    uint32_t _hostId = 0;
};

class LobbyManager {
   public:
    LobbyManager() = default;

    void onClientConnected(uint32_t clientId, const std::string& name = "Anonymous");
    void onClientDisconnected(uint32_t clientId);
    std::optional<std::reference_wrapper<ClientInfo>> getClient(uint32_t clientId);
    Lobby& createLobby(std::string name, uint32_t maxPlayers);
    bool joinLobby(uint32_t lobbyId, uint32_t clientId);
    bool leaveLobby(uint32_t clientId);
    std::optional<std::reference_wrapper<Lobby>> getLobby(uint32_t lobbyId);
    std::optional<std::reference_wrapper<Lobby>> getLobbyForClient(uint32_t clientId);
    const std::map<uint32_t, Lobby>& getAllLobbies() const { return _lobbies; }

   private:
    std::map<uint32_t, ClientInfo> _connectedClients;
    std::map<uint32_t, Lobby> _lobbies;
    std::map<uint32_t, uint32_t> _clientToLobbyMap;
    uint32_t _nextLobbyId = 1;
};

}  // namespace core
}  // namespace engine
