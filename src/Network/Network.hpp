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

struct chat_message {
    uint32_t sender_id;
    char sender_name[32];
    char message[256];
};

struct lobby_in_info {
    uint32_t id;
    char name[32];
    std::vector<uint32_t> id_player;
    uint32_t nbPlayers;
    uint32_t hostId;
};

struct AssignPlayerEntityPacket {
    uint32_t entityId;
};

// Serialization operators for AssignPlayerEntityPacket
// Serialization operators for AssignPlayerEntityPacket
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const AssignPlayerEntityPacket& packet) {
    msg << packet.entityId;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, AssignPlayerEntityPacket& packet) {
    msg >> packet.entityId;
    return msg;
}

// Serialization for lobby_info
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const lobby_info& info) {
    msg << info.id;
    msg.body.resize(msg.body.size() + 32);
    std::memcpy(msg.body.data() + msg.body.size() - 32, info.name, 32);
    msg.header.size = (uint32_t)msg.size();
    msg << info.nbConnectedPlayers;
    msg << info.maxPlayers;
    msg << info.state;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, lobby_info& info) {
    // Pop reverse of push: STATE, MAX, NB, NAME, ID
    msg >> info.state;
    msg >> info.maxPlayers;
    msg >> info.nbConnectedPlayers;

    // Manual pop for name (32 bytes)
    if (msg.body.size() < 32)
        throw std::runtime_error("Message too small for name");
    size_t i = msg.body.size() - 32;
    std::memcpy(info.name, msg.body.data() + i, 32);
    msg.body.resize(i);
    msg.header.size = (uint32_t)msg.size();

    msg >> info.id;
    return msg;
}

// Structure pour une entrée de score d'un joueur
struct PlayerScore {
    uint32_t client_id;
    int32_t score;
    bool is_alive;
};

// Structure pour le message S_GAME_OVER avec tous les scores
struct GameOverPacket {
    bool victory;  // true = victoire, false = défaite
    uint32_t player_count;
    PlayerScore players[8];  // Maximum 8 joueurs
};

// Serialization operators for GameOverPacket
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const GameOverPacket& packet) {
    msg << packet.victory;
    for (uint32_t i = 0; i < packet.player_count && i < 8; i++) {
        msg << packet.players[i].client_id;
        msg << packet.players[i].score;
        msg << packet.players[i].is_alive;
    }
    msg << packet.player_count;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, GameOverPacket& packet) {
    // CRITICAL: Message system is LIFO (Last In First Out)
    // Must read in REVERSE order of serialization

    // Lire d'abord player_count et victory (en ordre inverse)
    msg >> packet.player_count;

    // Sécurité: limiter player_count à 8 maximum
    if (packet.player_count > 8) {
        packet.player_count = 8;
    }

    // Lire UNIQUEMENT les joueurs qui ont été sérialisés (en ordre inverse)
    for (int i = static_cast<int>(packet.player_count) - 1; i >= 0; i--) {
        msg >> packet.players[i].is_alive;
        msg >> packet.players[i].score;
        msg >> packet.players[i].client_id;
    }

    msg >> packet.victory;

    return msg;
}

// Serialization for lobby_in_info
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const lobby_in_info& info) {
    msg << info.id;
    // Name
    msg.body.resize(msg.body.size() + 32);
    std::memcpy(msg.body.data() + msg.body.size() - 32, info.name, 32);
    msg.header.size = (uint32_t)msg.size();

    // Vector
    // Push elements first? or count first?
    // If pop is reverse:
    // Push: ID, NAME, VEC_ELEMENTS..., VEC_SIZE, HOST_ID, NB_PLAYERS

    for (uint32_t pid : info.id_player) {
        msg << pid;
    }
    msg << (uint32_t)info.id_player.size();  // vecSize
    msg << info.hostId;
    msg << info.nbPlayers;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, lobby_in_info& info) {
    // Pop reverse of push: NB_PLAYERS, HOST_ID, VEC_SIZE, VEC_ELEMENTS..., NAME, ID
    msg >> info.nbPlayers;

    msg >> info.hostId;

    uint32_t vecSize = 0;
    msg >> vecSize;

    info.id_player.resize(vecSize);
    // Pop elements (they were pushed in order 0..N, so they are at end in order N..0? No.
    // Pushed: 0, 1, 2...
    // Stack: ... 0, 1, 2.
    // Pop 2, then 1, then 0.
    // So we iterate reverse or fill reverse.
    for (int i = vecSize - 1; i >= 0; --i) {
        msg >> info.id_player[i];
    }

    // Pop name
    if (msg.body.size() < 32)
        throw std::runtime_error("Message too small for name");
    size_t i = msg.body.size() - 32;
    std::memcpy(info.name, msg.body.data() + i, 32);
    msg.body.resize(i);
    msg.header.size = (uint32_t)msg.size();

    msg >> info.id;
    return msg;
}

// Serialization for player
inline message<GameEvents>& operator<<(message<GameEvents>& msg, const player& p) {
    msg << p.id;
    msg.body.resize(msg.body.size() + 32);
    std::memcpy(msg.body.data() + msg.body.size() - 32, p.username, 32);
    msg.header.size = (uint32_t)msg.size();
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, player& p) {
    // Pop reverse: USERNAME, ID
    if (msg.body.size() < 32)
        throw std::runtime_error("Message too small for player username");
    size_t i = msg.body.size() - 32;
    std::memcpy(p.username, msg.body.data() + i, 32);
    msg.body.resize(i);
    msg.header.size = (uint32_t)msg.size();

    msg >> p.id;
    return msg;
}

// Voice packet structure for network transmission
struct voice_packet {
    uint32_t sender_id;
    uint32_t sequence_number;
    uint32_t timestamp;
    uint32_t data_size;
    uint8_t data[1024];  // Fixed size for easy serialization
};

}  // namespace network