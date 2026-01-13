#pragma once

#include <cstdint>
#include <functional>

#include "NetworkInterface/message.hpp"

namespace network {

enum class GameEvents : uint32_t {
    NONE,
    ASK_UDP,
    ASK_LOG,

    C_CONNECTION,
    S_SEND_ID,
    C_PING_SERVER,
    S_PING_SERVER,

    C_REGISTER,
    S_REGISTER_OK,
    S_REGISTER_KO,
    C_LOGIN,
    C_LOGIN_TOKEN,
    C_LOGIN_ANONYMOUS,
    S_INVALID_TOKEN,
    S_LOGIN_OK,
    S_LOGIN_KO,
    C_DISCONNECT,
    S_CONFIRM_UDP,
    C_CONFIRM_UDP,

    C_LIST_ROOMS,
    S_ROOMS_LIST,
    C_JOIN_ROOM,
    C_JOINT_RANDOM_LOBBY,
    S_ROOM_JOINED,
    S_PLAYER_JOINED,
    S_ROOM_NOT_JOINED,
    C_ROOM_LEAVE,
    S_PLAYER_LEAVE,
    S_ROOM_LEAVE,
    S_PLAYER_KICKED,
    S_ROOM_KICKED,
    S_NEW_HOST,
    C_NEW_LOBBY,
    S_CONFIRM_NEW_LOBBY,

    C_READY,
    S_READY_RETURN,
    C_GAME_START,
    S_GAME_START,
    S_GAME_START_KO,
    S_ASSIGN_PLAYER_ENTITY,
    C_CANCEL_READY,
    S_CANCEL_READY_BROADCAST,

    C_INPUT,
    S_SNAPSHOT,

    C_TEAM_CHAT,
    S_TEAM_CHAT,
    C_VOICE_PACKET,
    S_VOICE_RELAY,

    S_PLAYER_DEATH,
    S_ENTITY_DESTROY,
    S_SCORE_UPDATE,
    S_GAME_OVER,

    S_RETURN_TO_LOBBY,
};

// Hash function for GameEvents enum class
}  // namespace network
namespace std {
template <>
struct hash<network::GameEvents> {
    size_t operator()(network::GameEvents e) const noexcept { return static_cast<size_t>(e); }
};
}  // namespace std
namespace network {

struct coming_message {
    GameEvents id;
    uint32_t clientID;
    message<GameEvents> msg;
};

struct connection_info {
    char username[32];
    char password[32];
};

struct lobby_info {
    uint32_t id;
    char name[32];
    uint32_t nbConnectedPlayers;
    uint32_t maxPlayers;
    uint32_t state;
};
struct lobby_info_return {
    uint32_t nb_lobbys;
    std::vector<lobby_info> lobbies;
};

struct player {
    uint32_t id;
    char username[32];
};

struct lobby_in_info {
    uint32_t id;
    char name[32];
    std::vector<uint32_t> id_player;

    uint32_t nbPlayers;
};

struct AssignPlayerEntityPacket {
    uint32_t entityId;
};

// Serialization operators for AssignPlayerEntityPacket
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const AssignPlayerEntityPacket& packet) {
    msg << packet.entityId;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, AssignPlayerEntityPacket& packet) {
    msg >> packet.entityId;
    return msg;
}

// Structure pour une entrée de score d'un joueur
struct PlayerScore {
    uint32_t client_id;
    int32_t score;
    uint8_t is_alive;  // 0 = dead, 1 = alive (fixed size for network)
};

// Structure pour le message S_GAME_OVER avec tous les scores
struct GameOverPacket {
    uint8_t victory;  // 0 = défaite, 1 = victoire (fixed size for network)
    uint32_t player_count;
    PlayerScore players[8];  // Maximum 8 joueurs
};

// Serialization operators for GameOverPacket
// NOTE: Due to LIFO message system, we serialize player_count LAST so it can be read FIRST
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const GameOverPacket& packet) {
    // Write victory first (will be read last in LIFO)
    msg << packet.victory;
    
    // Write players data
    for (uint32_t i = 0; i < packet.player_count && i < 8; i++) {
        msg << packet.players[i].client_id;
        msg << packet.players[i].score;
        msg << packet.players[i].is_alive;
    }
    
    // Write player_count LAST (will be read FIRST in LIFO)
    msg << packet.player_count;
    
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, GameOverPacket& packet) {
    // LIFO: Read in reverse order of write
    
    // 1. Read player_count FIRST (was written last)
    msg >> packet.player_count;
    
    // Validate
    if (packet.player_count > 8) {
        packet.player_count = 8;
    }
    
    // 2. Read players in REVERSE order (last written player first)
    for (int i = static_cast<int>(packet.player_count) - 1; i >= 0; i--) {
        msg >> packet.players[i].is_alive;
        msg >> packet.players[i].score;
        msg >> packet.players[i].client_id;
    }
    
    // 3. Read victory LAST (was written first)
    msg >> packet.victory;
    
    return msg;
}

}  // namespace network