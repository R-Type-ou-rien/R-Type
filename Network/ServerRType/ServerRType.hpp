#pragma once
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../Database/Database.hpp"
#include "../Lobby/Lobby.hpp"
#include "../NetworkInterface/Connection.hpp"
#include "../NetworkInterface/ServerInterface.hpp"
#include "../NetworkRType.hpp"

#define DATABASE_FILE "rtype.db"
#define ALPHA_NUMERIC "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

#define MAX_PLAYERS 20

class ServerRType : public network::ServerInterface<RTypeEvents> {
    enum class ClientState : uint32_t { CONNECTED, WAITING_UDP_PING, LOGGED_IN, IN_LOBBY, READY, IN_GAME };

   public:
    ServerRType(uint16_t nPort, int timeout_seconds)
        : network::ServerInterface<RTypeEvents>(nPort), _timeout_seconds(timeout_seconds) {};

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
                if (event == RTypeEvents::S_RETURN_TO_LOBBY) {
                    _clientStates[client] = ClientState::IN_LOBBY;
                    client->SetTimeout(0);
                    return;
                }
                network::message<RTypeEvents> msg;
                msg.body << data;
                msg.header.id = event;
                msg.header.size = msg.size();
                if (std::find(_tcpEvents.begin(), _tcpEvents.end(), event) != _tcpEvents.end()) {
                    MessageClient(client, msg);
                } else if (std::find(_udpEvents.begin(), _udpEvents.end(), event) != _udpEvents.end()) {
                    MessageClientUDP(client, msg);
                } else {
                    MessageClient(client, msg);
                }

                break;
            }
        }
    }

    template <typename T>
    void AddMessageToLobby(RTypeEvents event, uint32_t id_lobby, const T& data) {
        for (Lobby<RTypeEvents>& lobby : _lobbys) {
            if (lobby.GetID() == id_lobby) {
                if (event == RTypeEvents::S_RETURN_TO_LOBBY) {
                    for (auto& [id, client] : lobby.getLobbyPlayers()) {
                        _clientStates[client] = ClientState::IN_LOBBY;
                        client->SetTimeout(0);
                        return;
                    }
                }
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

    std::vector<RTypeEvents> _tcpEvents = {
        RTypeEvents::C_PING_SERVER,
        RTypeEvents::C_REGISTER,
        RTypeEvents::S_REGISTER_OK,
        RTypeEvents::S_REGISTER_KO,
        RTypeEvents::C_LOGIN,
        RTypeEvents::C_LOGIN_TOKEN,
        RTypeEvents::S_INVALID_TOKEN,
        RTypeEvents::S_LOGIN_OK,
        RTypeEvents::S_LOGIN_KO,
        RTypeEvents::C_DISCONNECT,
        RTypeEvents::C_CONFIRM_UDP,
        RTypeEvents::C_LIST_ROOMS,
        RTypeEvents::S_ROOMS_LIST,
        RTypeEvents::C_JOIN_ROOM,
        RTypeEvents::S_ROOM_JOINED,
        RTypeEvents::S_PLAYER_JOINED,
        RTypeEvents::S_ROOM_NOT_JOINED,
        RTypeEvents::C_ROOM_LEAVE,
        RTypeEvents::S_PLAYER_LEAVE,
        RTypeEvents::S_PLAYER_KICKED,
        RTypeEvents::S_NEW_HOST,
        RTypeEvents::C_NEW_LOBBY,
        RTypeEvents::S_CONFIRM_NEW_LOBBY,
        RTypeEvents::C_READY,
        RTypeEvents::S_READY_RETURN,
        RTypeEvents::C_GAME_START,
        RTypeEvents::S_GAME_START,
        RTypeEvents::C_CANCEL_READY,
        RTypeEvents::S_CANCEL_READY_BROADCAST,
        RTypeEvents::C_QUIT_LOBBY,
        RTypeEvents::S_QUIT_LOBBY_BROADCAST,
        RTypeEvents::C_TEAM_CHAT,
        RTypeEvents::S_TEAM_CHAT,
        RTypeEvents::C_VOICE_PACKET,
        RTypeEvents::S_VOICE_RELAY,
        RTypeEvents::S_PLAYER_DEATH,
        RTypeEvents::S_SCORE_UPDATE,
        RTypeEvents::S_GAME_OVER,
        RTypeEvents::S_RETURN_TO_LOBBY,

    };

    std::vector<RTypeEvents> _udpEvents = {
        RTypeEvents::C_INPUT,
        RTypeEvents::S_SNAPSHOT,

    };

    std::queue<coming_message> _toGameMessages;

    Database _database{DATABASE_FILE};

    int _timeout_seconds = 30;
};
