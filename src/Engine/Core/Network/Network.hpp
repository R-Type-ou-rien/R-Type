#pragma once
#include <sys/types.h>

#include <cstdint>

#include "NetworkInterface/message.hpp"

enum class GameEvents : uint32_t {
    NONE,
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
    std::vector<player> players;
    uint32_t maxPlayers;
};