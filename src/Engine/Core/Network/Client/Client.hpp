#pragma once

#include <sys/types.h>

#include <cstdint>

#include "../NetworkInterface/ClientInterface.hpp"
#include "../Network.hpp"

#define TOKEN_FILENAME ".secrettoken"

class Client : public network::ClientInterface<GameEvents> {
   public:
    void PingServer();

    coming_message ReadIncomingMessage();

    void LoginServer(std::string username, std::string password);
    void LoginServerToken();
    void RegisterServer(std::string username, std::string password);

    template <typename T>
    void AddMessageToServer(GameEvents event, uint32_t id, const T& data) {
        network::message<GameEvents> msg;
        msg << data;
        msg.header.id = event;
        msg.header.user_id = _id;
        msg.header.size = msg.size();
        if (std::find(_tcpEvents.begin(), _tcpEvents.end(), event) != _tcpEvents.end()) {
            Send(msg);
        } else if (std::find(_udpEvents.begin(), _udpEvents.end(), event) != _udpEvents.end()) {
            SendUdp(msg);
        } else {
            Send(msg);
        }
    }

   private:
    uint32_t _id;

    std::vector<GameEvents> _tcpEvents = {
        GameEvents::C_PING_SERVER, GameEvents::S_SEND_ID,         GameEvents::C_REGISTER,
        GameEvents::S_REGISTER_OK, GameEvents::S_REGISTER_KO,     GameEvents::C_LOGIN,
        GameEvents::C_LOGIN_TOKEN, GameEvents::S_INVALID_TOKEN,   GameEvents::S_LOGIN_OK,
        GameEvents::S_LOGIN_KO,    GameEvents::C_DISCONNECT,      GameEvents::C_CONFIRM_UDP,
        GameEvents::C_LIST_ROOMS,  GameEvents::S_ROOMS_LIST,      GameEvents::C_JOIN_ROOM,
        GameEvents::S_ROOM_JOINED, GameEvents::S_PLAYER_JOINED,   GameEvents::S_ROOM_NOT_JOINED,
        GameEvents::C_ROOM_LEAVE,  GameEvents::S_PLAYER_LEAVE,    GameEvents::S_PLAYER_KICKED,
        GameEvents::S_NEW_HOST,    GameEvents::C_NEW_LOBBY,       GameEvents::S_CONFIRM_NEW_LOBBY,
        GameEvents::C_READY,       GameEvents::S_READY_RETURN,    GameEvents::C_GAME_START,
        GameEvents::S_GAME_START,  GameEvents::C_CANCEL_READY,    GameEvents::S_CANCEL_READY_BROADCAST,
        GameEvents::C_TEAM_CHAT,   GameEvents::S_TEAM_CHAT,       GameEvents::C_VOICE_PACKET,
        GameEvents::S_VOICE_RELAY, GameEvents::S_PLAYER_DEATH,    GameEvents::S_SCORE_UPDATE,
        GameEvents::S_GAME_OVER,   GameEvents::S_RETURN_TO_LOBBY, GameEvents::S_CONFIRM_UDP};

    std::vector<GameEvents> _udpEvents = {
        GameEvents::C_INPUT,
        GameEvents::S_SNAPSHOT,
        GameEvents::C_VOICE_PACKET,
        GameEvents::S_VOICE_RELAY,
    };
};
