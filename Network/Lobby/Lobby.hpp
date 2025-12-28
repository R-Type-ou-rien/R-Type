#pragma once
#include <cstdint>
#include <unordered_map>

#include "../NetworkInterface/Connection.hpp"
#include "../NetworkInterface/message.hpp"

#define MAX_LOBBY_PLAYERS 4

template <typename T>
class Lobby {
   public:
    Lobby(unsigned int id, std::string name) : _LobbyID(id), _name(name) {}

   public:
    enum class State : uint32_t {
        WAITING_FOR_PLAYERS,
        IN_GAME,
    };

    State GetState() const { return _state; }
    void SetState(State state) { _state = state; }

    std::unordered_map<unsigned int, std::shared_ptr<network::Connection<T>>> getLobbyPlayers() const {
        return _mapPlayers;
    }

    bool AddPlayer(std::shared_ptr<network::Connection<T>> client) {
        if (_mapPlayers.size() >= MAX_LOBBY_PLAYERS)
            return false;
        if (_state != State::WAITING_FOR_PLAYERS)
            return false;
        unsigned int id = client->GetID();
        _mapPlayers[id] = client;
        if (_mapPlayers.size() == 1) {
            _owner = client;
        }
        return true;
    }

    std::string GetName() const { return _name; };

    int GetNbPlayers() const { return _mapPlayers.size(); }

    int GetMaxPlayers() const { return MAX_LOBBY_PLAYERS; }

    void RemovePlayer(unsigned int clientID) { _mapPlayers.erase(clientID); }

    bool HasPlayer(unsigned int clientID) { return _mapPlayers.count(clientID) > 0; }

    void BroadcastMessage(const network::message<T>& msg) {
        for (auto& [id, connection] : _mapPlayers) {
            connection.Send(msg);
        }
    }

    std::shared_ptr<network::Connection<T>> getOwner() { return _owner; }
    void setOwner(std::shared_ptr<network::Connection<T>> owner) { _owner = owner; }

    unsigned int GetID() const { return _LobbyID; }

   private:
    State _state = State::WAITING_FOR_PLAYERS;
    unsigned int _LobbyID;
    std::string _name;
    std::unordered_map<unsigned int, std::shared_ptr<network::Connection<T>>> _mapPlayers;
    std::shared_ptr<network::Connection<T>> _owner;
};