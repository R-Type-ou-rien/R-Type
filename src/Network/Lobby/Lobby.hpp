#pragma once
#include <unordered_map>

#include "../NetworkInterface/Connection.hpp"
#include "../NetworkInterface/message.hpp"

template <typename T>
class Lobby {
   public:
    Lobby(unsigned int id) : _LobbyID(id) {}

   public:
    unsigned int GetID() const { return _LobbyID; }

    std::unordered_map<unsigned int, network::Connection<T>> getLobbyPlayers() const { return _mapPlayers; }

    void AddPlayer(network::Connection<T> client) {
        unsigned int id = client.GetID();
        _mapPlayers[id] = client;
    }

    void RemovePlayer(unsigned int clientID) { _mapPlayers.erase(clientID); }

    bool HasPlayer(unsigned int clientID) { return _mapPlayers.count(clientID) > 0; }

    void BroadcastMessage(const network::message<T>& msg) {
        for (auto& [id, connection] : _mapPlayers) {
            connection.Send(msg);
        }
    }

   private:
    unsigned int _LobbyID;
    std::unordered_map<unsigned int, network::Connection<T>> _mapPlayers;
};