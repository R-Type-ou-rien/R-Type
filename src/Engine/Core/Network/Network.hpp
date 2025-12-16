#pragma once
#include <sys/types.h>

#include <cstdint>

#include "NetworkInterface/message.hpp"

enum class GameEvents : uint32_t {
    NONE,
    CONNECTION_PLAYER,
    S_SEND_ID,
    C_PING_SERVER,
    S_PING_SERVER,

    C_REGISTER,
    S_REGISTER_OK,
    S_REGISTER_KO,
    C_LOGIN,
    C_LOGIN_TOKEN,
    S_INVALID_TOKEN,
    S_LOGIN_OK,
    S_LOGIN_KO,
    C_DISCONNECT,
    S_CONFIRM_UDP,
    C_CONFIRM_UDP,

    // LOBBY EVENTS
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

    // LANCEMENT
    C_READY,
    S_READY_RETURN,
    C_GAME_START,
    S_GAME_START,
    C_CANCEL_READY,
    S_CANCEL_READY_BROADCAST,

    // IN-GAME EVENTS
    C_INPUT,
    S_SNAPSHOT,

    // CHAT EVENTS
    C_TEAM_CHAT,
    S_TEAM_CHAT,
    C_VOICE_PACKET,
    S_VOICE_RELAY,

    // GAME EVENTS
    S_PLAYER_DEATH,
    S_SCORE_UPDATE,
    S_GAME_OVER,

    S_RETURN_TO_LOBBY,
};

struct coming_message {
    GameEvents id;
    uint32_t clientID;
    network::message<GameEvents> msg;
};

struct connection_info {
    char username[32];
    char password[32];
};

struct connection_server_return {
    std::string token;
    uint32_t id;
};

struct lobby_info {
    uint32_t id;
    std::string name;
    uint32_t ncConnectedPlayers;
    uint32_t maxPlayers;
};

struct player {
    uint32_t id;
    std::string username;
};

struct lobby_in_info {
    uint32_t id;
    std::string name;
    std::vector<uint32_t> id_player;
    // std::vector<player> players;
    uint32_t nbPlayers;
};

// Custom serialization for std::string
namespace network {
// Generic serialization for std::vector
template <typename T, typename U>
message<T>& operator<<(message<T>& msg, const std::vector<U>& vec) {
    for (const auto& item : vec)
        msg << item;
    msg << static_cast<uint32_t>(vec.size());
    return msg;
}

template <typename T, typename U>
message<T>& operator>>(message<T>& msg, std::vector<U>& vec) {
    uint32_t size = 0;
    msg >> size;
    vec.resize(size);
    for (int i = size - 1; i >= 0; --i)
        msg >> vec[i];
    return msg;
}

// Custom serialization for std::string
template <typename T>
message<T>& operator<<(message<T>& msg, const std::string& str) {
    uint32_t size = static_cast<uint32_t>(str.size());
    for (char c : str)
        msg << c;
    msg << size;
    return msg;
}

template <typename T>
message<T>& operator>>(message<T>& msg, std::string& str) {
    uint32_t size = 0;
    msg >> size;
    str.resize(size);
    for (int i = size - 1; i >= 0; --i)
        msg >> str[i];
    return msg;
}

// Custom serialization for player
template <typename T>
message<T>& operator<<(message<T>& msg, const ::player& p) {
    uint32_t nameSize = static_cast<uint32_t>(p.username.size());
    for (char c : p.username)
        msg << c;
    msg << nameSize;
    msg << p.id;
    return msg;
}

template <typename T>
message<T>& operator>>(message<T>& msg, ::player& p) {
    msg >> p.id;
    uint32_t nameSize = 0;
    msg >> nameSize;
    p.username.resize(nameSize);
    for (int i = nameSize - 1; i >= 0; --i)
        msg >> p.username[i];  // Pop reverse
    return msg;
}

// Custom serialization for lobby_info
template <typename T>
message<T>& operator<<(message<T>& msg, const ::lobby_info& info) {
    msg << info.maxPlayers;
    msg << info.ncConnectedPlayers;
    uint32_t nameSize = static_cast<uint32_t>(info.name.size());
    for (char c : info.name)
        msg << c;
    msg << nameSize;
    msg << info.id;
    return msg;
}

template <typename T>
message<T>& operator>>(message<T>& msg, ::lobby_info& info) {
    msg >> info.id;
    uint32_t nameSize = 0;
    msg >> nameSize;
    info.name.resize(nameSize);
    for (int i = nameSize - 1; i >= 0; --i)
        msg >> info.name[i];
    msg >> info.ncConnectedPlayers;
    msg >> info.maxPlayers;
    return msg;
}

// Custom serialization for lobby_in_info
template <typename T>
message<T>& operator<<(message<T>& msg, const ::lobby_in_info& info) {
    msg << info.nbPlayers;
    // Vector
    for (uint32_t pid : info.id_player)
        msg << pid;
    msg << static_cast<uint32_t>(info.id_player.size());
    // Name
    uint32_t nameSize = static_cast<uint32_t>(info.name.size());
    for (char c : info.name)
        msg << c;
    msg << nameSize;

    msg << info.id;
    return msg;
}

template <typename T>
message<T>& operator>>(message<T>& msg, ::lobby_in_info& info) {
    msg >> info.id;

    uint32_t nameSize = 0;
    msg >> nameSize;
    info.name.resize(nameSize);
    for (int i = nameSize - 1; i >= 0; --i)
        msg >> info.name[i];

    uint32_t vecSize = 0;
    msg >> vecSize;
    info.id_player.resize(vecSize);
    for (int i = vecSize - 1; i >= 0; --i)
        msg >> info.id_player[i];

    msg >> info.nbPlayers;
    return msg;
}

// Custom serialization for connection_server_return
template <typename T>
message<T>& operator<<(message<T>& msg, const ::connection_server_return& ret) {
    msg << ret.id;
    uint32_t tokenSize = static_cast<uint32_t>(ret.token.size());
    for (char c : ret.token)
        msg << c;
    msg << tokenSize;
    return msg;
}

template <typename T>
message<T>& operator>>(message<T>& msg, ::connection_server_return& ret) {
    uint32_t tokenSize = 0;
    msg >> tokenSize;
    ret.token.resize(tokenSize);
    for (int i = tokenSize - 1; i >= 0; --i)
        msg >> ret.token[i];
    msg >> ret.id;
    return msg;
}
}  // namespace network