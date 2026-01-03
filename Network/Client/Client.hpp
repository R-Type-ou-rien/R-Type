#pragma once

#include <cstdint>
#include <iostream>

#include "../NetworkInterface/ClientInterface.hpp"
#include "../Network.hpp"
#include "NetworkManager/NetworkManager.hpp"

#define TOKEN_FILENAME ".secrettoken"

namespace network {
class Client : public ClientInterface<GameEvents> {
   public:
    void PingServer();

    coming_message ReadIncomingMessage();

    void LoginServer(std::string username, std::string password);
    void LoginServerToken();
    void LoginAnonymous();
    void RegisterServer(std::string username, std::string password);

    template <typename T>
    void AddMessageToServer(GameEvents event, uint32_t id, const T& data) {
        network::message<GameEvents> msg;
        msg << data;
        SendValidatedMessage(event, msg);
    }

    void AddMessageToServer(GameEvents event, uint32_t id) {
        network::message<GameEvents> msg;
        SendValidatedMessage(event, msg);
    }

   private:
    template <typename T>
    void SendValidatedMessage(GameEvents event, network::message<T>& msg) {
        msg.header.id = event;
        msg.header.user_id = _id;
        msg.header.size = msg.size();

        // Validate packet before sending
        auto validation = _networkManager.validateServerPacket(event, msg);
        if (!validation.isValid()) {
            std::cerr << "[CLIENT] Packet validation failed: " << validation.errorMessage << std::endl;
            return;
        }

        if (_networkManager.isUdpEvent(event)) {
            SendUdp(msg);
        } else {
            Send(msg);
        }
    }

   private:
    uint32_t _id = 0;
    NetworkManager _networkManager;
};
}  // namespace network