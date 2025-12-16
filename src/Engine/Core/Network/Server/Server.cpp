#include "Server.hpp"

#include <sys/socket.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include "Network/Network.hpp"
#include "Network/NetworkInterface/message.hpp"

void Server::OnMessage(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents>& msg) {
    std::cout << "Event: " << (int)msg.header.id << std::endl;
    switch (msg.header.id) {
        case GameEvents::C_REGISTER:
            OnClientRegister(client, msg);
            break;
        case GameEvents::C_LOGIN:
            OnClientLogin(client, msg);
            break;
        case GameEvents::C_LOGIN_TOKEN:
            OnClientLoginToken(client, msg);
            break;
        case GameEvents::C_LIST_ROOMS:
            OnClientListLobby(client, msg);
            break;
        case GameEvents::C_JOIN_ROOM:
            OnClientJoinLobby(client, msg);
            break;
        case GameEvents::C_ROOM_LEAVE:
            OnClientLeaveLobby(client, msg);
            break;
        case GameEvents::C_NEW_LOBBY:
            OnClientNewLobby(client, msg);
            break;
        case GameEvents::C_CONFIRM_UDP:
            // if (_clientStates[client] == ClientState::WAITING_UDP_PING)
            //     _clientStates[client] = ClientState::LOGGED_IN;
            if (_clientStates[client] == ClientState::WAITING_UDP_PING) {
                _lobbys.back().AddPlayer(client);
                AddMessageToLobby(GameEvents::S_PLAYER_JOINED, _lobbys.back().GetID(), client->GetID());
                struct lobby_in_info info;
                info.id = _lobbys.back().GetID();
                info.name = _lobbys.back().GetName();
                info.nbPlayers = _lobbys.back().GetNbPlayers();
                for (auto& [id, connection] : _lobbys.back().getLobbyPlayers())
                    info.id_player.push_back(id);

                AddMessageToPlayer(GameEvents::S_ROOM_JOINED, client->GetID(), info);

                _clientStates[client] = ClientState::IN_LOBBY;
                if (_lobbys.back().GetNbPlayers() >= 2) {
                    for (auto& [id, connection] : _lobbys.back().getLobbyPlayers()) {
                        if (_clientStates[connection] != ClientState::IN_LOBBY)
                            return;
                    }
                    for (auto& [id, connection] : _lobbys.back().getLobbyPlayers())
                        _clientStates[connection] = ClientState::IN_GAME;
                    AddMessageToLobby(GameEvents::S_GAME_START, _lobbys.back().GetID(), nullptr);
                    _toGameMessages.push(
                        {GameEvents::C_GAME_START, _lobbys.back().GetID(), network::message<GameEvents>()});
                }
            }
            break;
        default:
            _toGameMessages.push({msg.header.id, msg.header.user_id, msg});
            break;
    }
}

void Server::OnClientDisconnect(std::shared_ptr<network::Connection<GameEvents>> client) {
    // std::cout << "Removing client [" << client->GetID() << "]\n";
    network::message<GameEvents> msg;
    msg << NULL;
    _toGameMessages.push({GameEvents::C_DISCONNECT, client->GetID(), msg});
}

bool Server::OnClientConnect(std::shared_ptr<network::Connection<GameEvents>> client) {
    if (_deqConnections.size() > _maxConnections)
        return false;
    client->SetTimeout(0);
    // AddMessageToPlayer(GameEvents::C_PING_SERVER, client->GetID(), NULL);

    network::message<GameEvents> msg;
    msg << client->GetID();
    _toGameMessages.push({GameEvents::CONNECTION_PLAYER, client->GetID(), msg});
    std::cout << "envoie de l'id" << std::endl;

    _clientStates[client] = ClientState::WAITING_UDP_PING;
    return true;
}

coming_message Server::ReadIncomingMessage() {
    if (_toGameMessages.empty())
        return coming_message{};

    coming_message message = _toGameMessages.front();
    _toGameMessages.pop();
    return message;
}

void Server::OnClientRegister(std::shared_ptr<network::Connection<GameEvents>> client,
                              network::message<GameEvents> msg) {
    connection_info info;
    msg >> info;

    _database.RegisterUser(info.username, info.password);

    connection_server_return returnInfo;
    const std::string charset = ALPHA_NUMERIC;
    srand(time(nullptr) + client->GetID());
    for (int i = 0; i < 10; i++) {
        returnInfo.token += charset[rand() % charset.length()];
    }
    _database.SaveToken(_database.LoginUser(info.username, info.password), returnInfo.token);
    returnInfo.id = client->GetID();

    _clientUsernames[client] = info.username;
    _clientStates[client] = ClientState::WAITING_UDP_PING;
    AddMessageToPlayer(GameEvents::S_REGISTER_OK, client->GetID(), returnInfo);
}

void Server::OnClientLogin(std::shared_ptr<network::Connection<GameEvents>> client, network::message<GameEvents> msg) {
    connection_info info;
    msg >> info;

    int userID = _database.LoginUser(info.username, info.password);
    if (userID == -1) {
        AddMessageToPlayer(GameEvents::S_LOGIN_KO, client->GetID(), NULL);
        return;
    }
    connection_server_return returnInfo;

    returnInfo.token = _database.GetTokenById(userID);
    returnInfo.id = client->GetID();

    _clientUsernames[client] = info.username;
    _clientStates[client] = ClientState::WAITING_UDP_PING;
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), returnInfo);
}

void Server::OnClientLoginToken(std::shared_ptr<network::Connection<GameEvents>> client,
                                network::message<GameEvents> msg) {
    std::string token;
    msg >> token;

    int userID = _database.GetUserByToken(token);
    if (userID == -1) {
        AddMessageToPlayer(GameEvents::S_INVALID_TOKEN, client->GetID(), NULL);
        return;
    }
    connection_server_return returnInfo;
    returnInfo.id = client->GetID();
    returnInfo.token = _database.GetTokenById(userID);
    if (returnInfo.token != token) {
        AddMessageToPlayer(GameEvents::S_INVALID_TOKEN, client->GetID(), returnInfo);
    }

    returnInfo.id = client->GetID();

    //_clientUsernames[client] = username;
    _clientStates[client] = ClientState::WAITING_UDP_PING;
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), returnInfo);
}

void Server::OnClientListLobby(std::shared_ptr<network::Connection<GameEvents>> client,
                               network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN)
        return;
    lobby_info info;
    std::vector<lobby_info> lobbysInfo;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        info.id = lobby.GetID();
        info.name = lobby.GetName();
        info.ncConnectedPlayers = lobby.GetNbPlayers();
        info.maxPlayers = lobby.GetMaxPlayers();
        lobbysInfo.push_back(info);
    }
    AddMessageToPlayer(GameEvents::S_ROOMS_LIST, client->GetID(), lobbysInfo);
}

void Server::OnClientJoinLobby(std::shared_ptr<network::Connection<GameEvents>> client,
                               network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN)
        return;
    uint32_t lobbyID;
    msg >> lobbyID;
    bool lobbyFound = false;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.GetID() == lobbyID) {
            lobby.AddPlayer(client);
            lobbyFound = true;

            struct lobby_in_info info;
            info.id = lobby.GetID();
            info.name = lobby.GetName();
            info.nbPlayers = lobby.GetNbPlayers();
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                // player p;
                // p.id = id;
                // p.username = _clientUsernames[connection];
                info.id_player.push_back(id);
                // info.players.push_back(p);
            }
            AddMessageToPlayer(GameEvents::S_ROOM_JOINED, client->GetID(), info);

            struct player p;
            p.id = client->GetID();
            p.username = _clientUsernames[client];
            AddMessageToLobby(GameEvents::S_PLAYER_JOINED, lobbyID, p);
            _toGameMessages.push({GameEvents::S_PLAYER_JOINED, client->GetID(), msg});

            _clientStates[client] = ClientState::IN_LOBBY;
            break;
        }
    }
}
void Server::OnClientLeaveLobby(std::shared_ptr<network::Connection<GameEvents>> client,
                                network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::IN_LOBBY)
        return;
    uint32_t lobbyID;
    msg >> lobbyID;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        auto mapPlayers = lobby.getLobbyPlayers();
        if (lobby.GetID() == lobbyID) {
            _clientStates[client] = ClientState::LOGGED_IN;
            if (mapPlayers[client->GetID()] == lobby.getOwner()) {
                if (mapPlayers.size() > 1) {
                    for (auto& [id, connection] : mapPlayers) {
                        if (id != client->GetID()) {
                            lobby.setOwner(connection);
                            AddMessageToLobby(GameEvents::S_NEW_HOST, lobbyID, id);
                            break;
                        }
                    }
                } else {
                    _lobbys.erase(
                        std::remove_if(_lobbys.begin(), _lobbys.end(),
                                       [lobbyID](const Lobby<GameEvents>& lobby) { return lobby.GetID() == lobbyID; }),
                        _lobbys.end());
                    return;
                }
            }
            lobby.RemovePlayer(client->GetID());
            struct player p;
            p.id = client->GetID();
            p.username = _clientUsernames[client];
            AddMessageToLobby(GameEvents::S_PLAYER_LEAVE, lobbyID, p);
            _toGameMessages.push({GameEvents::S_PLAYER_LEAVE, client->GetID(), msg});

            break;
        }
    }
}

void Server::OnClientNewLobby(std::shared_ptr<network::Connection<GameEvents>> client,
                              network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN)
        return;
    std::string lobbyName;
    msg >> lobbyName;
    uint32_t newLobbyID = 1;
    if (!_lobbys.empty()) {
        newLobbyID = _lobbys.back().GetID() + 1;
    }
    Lobby<GameEvents> newLobby(newLobbyID, lobbyName);

    _lobbys.emplace_back(newLobby);
    _lobbys.back().AddPlayer(client);
    AddMessageToPlayer(GameEvents::S_CONFIRM_NEW_LOBBY, client->GetID(), newLobby.GetName());
    _clientStates[client] = ClientState::IN_LOBBY;
}

void Server::onClientStartGame(std::shared_ptr<network::Connection<GameEvents>> client,
                               network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::READY)
        return;
    uint32_t info;
    msg >> info;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.GetID() == info) {
            if (lobby.getOwner() != client) {
                return;
            }
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                if (_clientStates[connection] != ClientState::READY) {
                    return;
                }
            }
            _toGameMessages.push({GameEvents::S_GAME_START, client->GetID(), msg});
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                _clientStates[connection] = ClientState::IN_GAME;
                connection->SetTimeout(_timeout_seconds);
            }
            AddMessageToLobby(GameEvents::S_GAME_START, lobby.GetID(), NULL);
            break;
        }
    }
}

void Server::onClientReadyUp(std::shared_ptr<network::Connection<GameEvents>> client,
                             network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::IN_LOBBY)
        return;
    uint32_t info;
    msg >> info;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.GetID() == info) {
            _clientStates[client] = ClientState::READY;
            struct player p;
            p.id = client->GetID();
            p.username = _clientUsernames[client];
            AddMessageToLobby(GameEvents::S_READY_RETURN, lobby.GetID(), p);
            break;
        }
    }
}
void Server::onClientUnready(std::shared_ptr<network::Connection<GameEvents>> client,
                             network::message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::READY)
        return;
    uint32_t info;
    msg >> info;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.GetID() == info) {
            _clientStates[client] = ClientState::IN_LOBBY;
            struct player p;
            p.id = client->GetID();
            p.username = _clientUsernames[client];
            AddMessageToLobby(GameEvents::S_CANCEL_READY_BROADCAST, lobby.GetID(), p);
            break;
        }
    }
}
