#pragma once
#include <memory>
#include <vector>

#include "../Lobby/Lobby.hpp"
#include "../Network.hpp"
#include "../NetworkInterface/Connection.hpp"
#include "../NetworkInterface/ServerInterface.hpp"

class ServerRType : public network::ServerInterface<RTypeEvents> {
   public:
    ServerRType(uint16_t nPort) : network::ServerInterface<RTypeEvents>(nPort) {}

   protected:
    virtual bool OnClientConnect(std::shared_ptr<network::Connection<RTypeEvents>> client);

    virtual void OnClientDisconnect(std::shared_ptr<network::Connection<RTypeEvents>> client);

    virtual void OnMessage(std::shared_ptr<network::Connection<RTypeEvents>> client,
                           network::message<RTypeEvents>& msg);

    template <typename T>
    coming_message<T> ReadIncomingMessage() {
        // il faut coder ici
    };

    template <typename T>
    void AddMessageToServer(RTypeEvents event, uint32_t id, const T& data) {
        // il faut coder ici
    }

   private:
    std::vector<Lobby<RTypeEvents>> clientIDs;
};
