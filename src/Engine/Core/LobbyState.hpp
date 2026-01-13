#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace engine::core {

struct LobbyPlayerInfo {
    uint32_t id = 0;
    std::string name;
    bool isReady = false;
    bool isHost = false;
};

struct LobbyState {
    uint32_t lobbyId = 0;
    std::string lobbyName;
    std::vector<LobbyPlayerInfo> players;
    uint32_t hostId = 0;
    uint32_t localClientId = 0;
    bool localPlayerReady = false;

    bool isLocalPlayerHost() const { return hostId == localClientId && localClientId != 0; }

    bool areAllPlayersReady() const {
        if (players.empty())
            return false;
        for (const auto& p : players) {
            if (!p.isReady)
                return false;
        }
        return true;
    }

    bool canStartGame() const { return isLocalPlayerHost() && areAllPlayersReady(); }

    void setPlayerReady(uint32_t clientId, bool ready) {
        for (auto& p : players) {
            if (p.id == clientId) {
                p.isReady = ready;
                break;
            }
        }
        if (clientId == localClientId) {
            localPlayerReady = ready;
        }
    }

    void reset() {
        lobbyId = 0;
        lobbyName.clear();
        players.clear();
        hostId = 0;
        localPlayerReady = false;
    }
};

}  // namespace engine::core
