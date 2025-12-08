#pragma once
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../Lobby/Lobby.hpp"
#include "../NetworkInterface/Connection.hpp"
#include "../NetworkInterface/ServerInterface.hpp"
#include "../NetworkRType.hpp"

#define MAX_PLAYERS 20

class ServerRType : public network::ServerInterface<RTypeEvents> {
    enum class ClientState : uint32_t { CONNECTED, WAITING_UDP_PING, LOGGED_IN, IN_LOBBY, READY, IN_GAME };

   public:
    ServerRType(uint16_t nPort) : network::ServerInterface<RTypeEvents>(nPort) {}

   protected:
    virtual void OnMessage(std::shared_ptr<network::Connection<RTypeEvents>> client,
                           network::message<RTypeEvents>& msg);

    virtual bool OnClientConnect(std::shared_ptr<network::Connection<RTypeEvents>> client);
    virtual void OnClientDisconnect(std::shared_ptr<network::Connection<RTypeEvents>> client);

    // Connection and Lobby event handlers (croyez pas y'a que gemini qui sait faire des commentaires bandes de fous)
    void OnClientRegister(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);
    void OnClientLogin(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);
    void OnClientLoginToken(std::shared_ptr<network::Connection<RTypeEvents>> client,
                            network::message<RTypeEvents> msg);
    void OnClientListLobby(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);
    void OnClientJoinLobby(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);
    void OnClientLeaveLobby(std::shared_ptr<network::Connection<RTypeEvents>> client,
                            network::message<RTypeEvents> msg);
    void OnClientNewLobby(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);

    // Pre-Game event handlers
    void onClientStartGame(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);
    void onClientReadyUp(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);
    void onClientUnready(std::shared_ptr<network::Connection<RTypeEvents>> client, network::message<RTypeEvents> msg);

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
    int _maxConnections = MAX_PLAYERS;

    std::vector<Lobby<RTypeEvents>> _lobbys;
    std::unordered_map<std::shared_ptr<network::Connection<RTypeEvents>>, ClientState> _clientStates;
    std::unordered_map<std::shared_ptr<network::Connection<RTypeEvents>>, std::string> _clientUsernames;

    std::vector<RTypeEvents> _udpEvents;
    std::vector<RTypeEvents> _tcpEvents;

    std::queue<coming_message> _toGameMessages;
};
