#include "LobbyManager.hpp"
#include <algorithm>
#include <utility>

namespace engine {
namespace core {

// Lobby methods
bool Lobby::addClient(const ClientInfo& client) {
    if (isFull()) {
        return false;
    }
    auto it = std::find_if(_clients.begin(), _clients.end(), [&](const ClientInfo& c) { return c.id == client.id; });
    if (it == _clients.end()) {
        _clients.push_back(client);
        return true;
    }
    return false;  // Client already in lobby
}

bool Lobby::removeClient(uint32_t clientId) {
    auto it = std::remove_if(_clients.begin(), _clients.end(),
                             [&](const ClientInfo& client) { return client.id == clientId; });
    if (it != _clients.end()) {
        _clients.erase(it, _clients.end());
        return true;
    }
    return false;
}

// LobbyManager methods
void LobbyManager::onClientConnected(uint32_t clientId, const std::string& name) {
    _connectedClients[clientId] = {clientId, name};
}

void LobbyManager::onClientDisconnected(uint32_t clientId) {
    leaveLobby(clientId);
    _connectedClients.erase(clientId);
}

std::optional<std::reference_wrapper<ClientInfo>> LobbyManager::getClient(uint32_t clientId) {
    auto it = _connectedClients.find(clientId);
    if (it != _connectedClients.end()) {
        return it->second;
    }
    return std::nullopt;
}

Lobby& LobbyManager::createLobby(std::string name, uint32_t maxPlayers) {
    uint32_t lobbyId = _nextLobbyId++;
    _lobbies.emplace(lobbyId, Lobby(lobbyId, std::move(name), maxPlayers));
    return _lobbies.at(lobbyId);
}

bool LobbyManager::joinLobby(uint32_t lobbyId, uint32_t clientId) {
    auto lobbyIt = _lobbies.find(lobbyId);
    if (lobbyIt == _lobbies.end()) {
        return false;  // Lobby not found
    }

    auto clientIt = _connectedClients.find(clientId);
    if (clientIt == _connectedClients.end()) {
        return false;  // Client not found
    }

    // If client is already in another lobby, leave it first
    leaveLobby(clientId);

    if (lobbyIt->second.addClient(clientIt->second)) {
        _clientToLobbyMap[clientId] = lobbyId;
        return true;
    }

    return false;  // Lobby is full or client already in it
}

bool LobbyManager::leaveLobby(uint32_t clientId) {
    auto mappingIt = _clientToLobbyMap.find(clientId);
    if (mappingIt != _clientToLobbyMap.end()) {
        uint32_t lobbyId = mappingIt->second;
        auto lobbyIt = _lobbies.find(lobbyId);
        if (lobbyIt != _lobbies.end()) {
            lobbyIt->second.removeClient(clientId);
        }
        _clientToLobbyMap.erase(mappingIt);
        return true;
    }
    return false;  // Client was not in any lobby
}

std::optional<std::reference_wrapper<Lobby>> LobbyManager::getLobby(uint32_t lobbyId) {
    auto it = _lobbies.find(lobbyId);
    if (it != _lobbies.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::reference_wrapper<Lobby>> LobbyManager::getLobbyForClient(uint32_t clientId) {
    auto it = _clientToLobbyMap.find(clientId);
    if (it != _clientToLobbyMap.end()) {
        return getLobby(it->second);
    }
    return std::nullopt;
}

}  // namespace core
}  // namespace engine
