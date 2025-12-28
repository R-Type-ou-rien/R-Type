#pragma once

#include <cstdint>

#include "../NetworkInterface/ClientInterface.hpp"
#include "../Network.hpp"

#define TOKEN_FILENAME ".secrettoken"

namespace network {
class Client : public ClientInterface<GameEvents> {
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
        if (std::find(_udpEvents.begin(), _udpEvents.end(), event) != _udpEvents.end()) {
            SendUdp(msg);
        } else {
            Send(msg);
        }
    }

   private:
    uint32_t _id = 0;
    std::vector<GameEvents> _udpEvents = {
        GameEvents::S_SNAPSHOT,
        GameEvents::C_INPUT,

        GameEvents::C_VOICE_PACKET,
        GameEvents::S_VOICE_RELAY,
    };
};
}  // namespace network