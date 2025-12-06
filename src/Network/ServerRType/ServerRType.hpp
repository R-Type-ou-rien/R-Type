#pragma once
#include <functional>
#include <map>
#include <memory>
#include <queue>
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

    coming_message ReadIncomingMessage();

    template <typename T>
    void AddMessageToPlayer(RTypeEvents event, uint32_t id, const T& data) {
        for (std::shared_ptr<network::Connection<RTypeEvents>>& client : _deqConnections) {
            if (client->GetID() == id) {
                network::message<RTypeEvents> msg;
                msg.body << data;
                msg.header.id = event;
                msg.header.size = msg.size();
                if (std::find(_tcpEvents.begin(), _tcpEvents.end(), event) != _tcpEvents.end()) {
                    MessageClient(client, msg);
                } else if (std::find(_udpEvents.begin(), _udpEvents.end(), event) != _udpEvents.end()) {
                    MessageClientUDP(client, msg);
                }
                break;
            }
        }
    }

    template <typename T>
    void AddMessageToLobby(RTypeEvents event, uint32_t id_lobby, const T& data) {
        for (Lobby<RTypeEvents>& lobby : _lobbys) {
            if (lobby.GetID() == id_lobby) {
                lobby.BroadcastMessage([&]() {
                    network::message<RTypeEvents> msg;
                    msg.body << data;
                    msg.header.id = event;
                    msg.header.size = msg.size();
                    return msg;
                }());
                break;
            }
        }
    }

   private:
    std::vector<Lobby<RTypeEvents>> _lobbys;
    std::vector<RTypeEvents> _udpEvents;
    std::vector<RTypeEvents> _tcpEvents;

    std::queue<coming_message> _toGameMessages;
};
