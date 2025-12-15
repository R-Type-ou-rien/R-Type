#pragma once

#include <sys/types.h>

#include <cstdint>

#include "../NetworkInterface/ClientInterface.hpp"
#include "../Network.hpp"

#define TOKEN_FILENAME ".secrettoken"

class Client : public network::ClientInterface<RTypeEvents> {
   public:
    void PingServer();

    coming_message ReadIncomingMessage();

    void LoginServer(std::string username, std::string password);
    void LoginServerToken();
    void RegisterServer(std::string username, std::string password);

    template <typename T>
    void AddMessageToServer(RTypeEvents event, uint32_t id, const T& data) {
        network::message<RTypeEvents> msg;
        msg.body << data;
        msg.header.id = event;
        msg.header.user_id = _id;
        msg.header.size = msg.size();
        Send(msg);
    }

   private:
    uint32_t _id;
};
