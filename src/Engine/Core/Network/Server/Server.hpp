#pragma once
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../Database/Database.hpp"
#include "../Lobby/Lobby.hpp"
#include "../NetworkInterface/Connection.hpp"
#include "../NetworkInterface/ServerInterface.hpp"
#include "../Network.hpp"

#define DATABASE_FILE "rtype.db"
#define ALPHA_NUMERIC "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

#define MAX_PLAYERS 20

class Server : public network::ServerInterface<GameEvents> {
    enum class ClientState : uint32_t { CONNECTED, WAITING_UDP_PING, LOGGED_IN, IN_LOBBY, READY, IN_GAME };

   public:
    Server(uint16_t nPort, int timeout_seconds)
        : network::ServerInterface<GameEvents>(nPort), _timeout_seconds(timeout_seconds) {};

   protected:
    virtual void OnMessage(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents>& msg);

    virtual bool OnClientConnect(std::shared_ptr<network::Connection<GameEvents>> client);
    virtual void OnClientDisconnect(std::shared_ptr<network::Connection<GameEvents>> client);

    // Connection and Lobby event handlers (croyez pas y'a que gemini qui sait faire des commentaires bandes de fous)
    void OnClientRegister(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void OnClientLogin(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void OnClientLoginToken(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void OnClientListLobby(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void OnClientJoinLobby(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void OnClientLeaveLobby(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void OnClientNewLobby(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);

    // Pre-Game event handlers
    void onClientStartGame(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void onClientReadyUp(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);
    void onClientUnready(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg);

    coming_message ReadIncomingMessage();

    template <typename T>
    void AddMessageToPlayer(GameEvents event, uint32_t id, const T& data) {
        for (std::shared_ptr<network::Connection<GameEvents>>& client : _deqConnections) {
            if (client->GetID() == id) {
                if (event == GameEvents::S_RETURN_TO_LOBBY) {
                    _clientStates[client] = ClientState::IN_LOBBY;
                    client->SetTimeout(0);
                    return;
                }
                network::message<GameEvents> msg;
                msg << data;
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
    void AddMessageToLobby(GameEvents event, uint32_t id_lobby, const T& data) {
        for (Lobby<GameEvents>& lobby : _lobbys) {
            if (lobby.GetID() == id_lobby) {
                if (event == GameEvents::S_RETURN_TO_LOBBY) {
                    for (auto& [id, client] : lobby.getLobbyPlayers()) {
                        _clientStates[client] = ClientState::IN_LOBBY;
                        client->SetTimeout(0);
                        return;
                    }
                }
                lobby.BroadcastMessage([&]() {
                    network::message<GameEvents> msg;
                    msg << data;
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

    std::vector<Lobby<GameEvents>> _lobbys;
    std::unordered_map<std::shared_ptr<network::Connection<GameEvents>>, ClientState> _clientStates;
    std::unordered_map<std::shared_ptr<network::Connection<GameEvents>>, std::string> _clientUsernames;

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

    std::queue<coming_message> _toGameMessages;

    Database _database{DATABASE_FILE};

    int _timeout_seconds = 30;
};
