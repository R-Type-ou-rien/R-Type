#pragma once
#include <cstdint>
#include <vector>

#include "NetworkInterface/message.hpp"

enum class RTypeEvents : uint32_t {
    C_REGISTER,
    S_REGISTER_OK,
    S_REGISTER_KO,
    C_LOGIN,
    S_LOGIN_OK,
    S_LOGIN_KO,
    C_DISCONNECT,

    // LOBBY EVENTS
    C_LIST_ROOMS,
    S_ROOMS_LIST,
    C_JOIN_ROOM,
    S_ROOM_JOINED,
    S_PLAYER_JOINED,
    S_ROOM_NOT_JOINED,
    C_ROOM_LEAVE,
    S_PLAYER_LEAVE,
    S_PLAYER_KICKED,
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
    C_QUIT_LOBBY,
    S_QUIT_LOBBY_BROADCAST,

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
    S_GAME_OVER
};

struct coming_message {
    RTypeEvents id;
    network::message<RTypeEvents> msg;
};
