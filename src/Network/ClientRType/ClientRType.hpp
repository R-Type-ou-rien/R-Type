#pragma once

#include <sys/types.h>

#include "../Network.hpp"
#include "../NetworkInterface/ClientInterface.hpp"

class ClientRType : public network::ClientInterface<RTypeEvents> {
   public:
    void PingServer();

    void MessageAll();

    template <typename T>
    coming_message<T> ReadIncomingMessage() {
        // il faut coder ici
    };

    template <typename T>
    void AddMessageToServer(RTypeEvents event, uint32_t id, const T& data) {
        // il faut coder ici
    }
};
