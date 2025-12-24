#include "Server.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

using namespace network;

void Server::OnMessage(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents>& msg) {
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
        case GameEvents::C_JOINT_RANDOM_LOBBY:
            OnClientJoinRandomLobby(client, msg);
            break;
        case GameEvents::C_ROOM_LEAVE:
            OnClientLeaveLobby(client, msg);
            break;
        case GameEvents::C_NEW_LOBBY:
            OnClientNewLobby(client, msg);
            break;
        case GameEvents::C_CONFIRM_UDP:
            if (_clientStates[client] == ClientState::WAITING_UDP_PING)
                _clientStates[client] = ClientState::CONNECTED;
            break;
        default:
            _toGameMessages.push({msg.header.id, msg.header.user_id, msg});
            break;
    }
}

void Server::OnClientDisconnect(std::shared_ptr<Connection<GameEvents>> client) {
    std::cout << "Removing client [" << client->GetID() << "]\n";
    message<GameEvents> msg;
    msg << NULL;
    _toGameMessages.push({GameEvents::C_DISCONNECT, client->GetID(), msg});
}

bool Server::OnClientConnect(std::shared_ptr<Connection<GameEvents>> client) {
    if (_deqConnections.size() >= _maxConnections)
        return false;
    client->SetTimeout(0);

    // Confirmation connection (TOUJOURS PAS UN COMMENTAIRE DE GEMINI OU AUTRE)
    AddMessageToPlayer(GameEvents::S_SEND_ID, client->GetID(), NULL);
    // Demander un paquet UDP pour save le endpint  udp (eh vsy j'ai meme pas besoin de parler la)
    AddMessageToPlayer(GameEvents::S_CONFIRM_UDP, client->GetID(), NULL);

    // Envoi du message de connection au game (bON POUR CA L'AUTOCOMPLETION DE L'IDE A UN PEU AIDER)
    network::message<GameEvents> msg;
    msg << NULL;
    _toGameMessages.push({GameEvents::C_CONNECTION, client->GetID(), msg});

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

void Server::OnClientRegister(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    connection_info info;
    msg >> info;

    if (_clientStates[client] != ClientState::CONNECTED) {
        AddMessageToPlayer(GameEvents::ASK_UDP, client->GetID(), NULL);
        return;
    }
    _database.RegisterUser(info.username, info.password);

    std::string token = "";
    const std::string charset = ALPHA_NUMERIC;
    srand(time(nullptr) + client->GetID());
    for (int i = 0; i < 10; i++) {
        token += charset[rand() % charset.length()];
    }
    _database.SaveToken(_database.LoginUser(info.username, info.password), token);

    _clientUsernames[client] = info.username;
    _clientStates[client] = ClientState::LOGGED_IN;
    AddMessageToPlayer(GameEvents::S_REGISTER_OK, client->GetID(), token);
}

void Server::OnClientLogin(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    connection_info info;
    msg >> info;

    if (_clientStates[client] != ClientState::CONNECTED) {
        AddMessageToPlayer(GameEvents::ASK_UDP, client->GetID(), NULL);
        return;
    }
    int userID = _database.LoginUser(info.username, info.password);
    if (userID == -1) {
        AddMessageToPlayer(GameEvents::S_LOGIN_KO, client->GetID(), NULL);
        return;
    }

    std::string token = _database.GetTokenById(userID);
    _clientUsernames[client] = info.username;
    _clientStates[client] = ClientState::LOGGED_IN;
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), token);
}

void Server::OnClientLoginToken(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    std::string token;
    msg >> token;

    if (_clientStates[client] != ClientState::CONNECTED) {
        AddMessageToPlayer(GameEvents::ASK_UDP, client->GetID(), NULL);
        return;
    }
    int userID = _database.GetUserByToken(token);
    if (userID == -1) {
        AddMessageToPlayer(GameEvents::S_INVALID_TOKEN, client->GetID(), NULL);
        return;
    }

    _clientUsernames[client] = _database.GetNameById(userID);
    _clientStates[client] = ClientState::LOGGED_IN;
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), NULL);
}

void Server::OnClientListLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN) {
        AddMessageToPlayer(GameEvents::ASK_LOG, client->GetID(), NULL);
        return;
    }
    lobby_info_return return_info;
    return_info.nb_lobbys = _lobbys.size();

    std::vector<lobby_info> lobbysInfo;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        lobby_info info;
        info.id = lobby.GetID();
        info.name = lobby.GetName();
        info.ncConnectedPlayers = lobby.GetNbPlayers();
        info.maxPlayers = lobby.GetMaxPlayers();
        info.state = (uint32_t)lobby.GetState();
        lobbysInfo.push_back(info);
    }
    return_info.lobbies = lobbysInfo;
    AddMessageToPlayer(GameEvents::S_ROOMS_LIST, client->GetID(), return_info);
}

void Server::OnClientJoinLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN) {
        AddMessageToPlayer(GameEvents::ASK_LOG, client->GetID(), NULL);
        return;
    }
    uint32_t lobbyID;
    msg >> lobbyID;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.GetID() == lobbyID) {
            if (!lobby.AddPlayer(client)) {
                AddMessageToPlayer(GameEvents::S_ROOM_NOT_JOINED, client->GetID(), NULL);
                return;
            }

            // envoyer le message de player joined a tous les joueurs du lobby (ca aussi c vignesh MOUROUGANANDAME QUI A
            // ECRIT LE COMMENTAIRE BORIS JE VAIS TE HAGAR)
            struct player p;
            p.id = client->GetID();
            p.username = _clientUsernames[client];
            AddMessageToLobby(GameEvents::S_PLAYER_JOINED, lobbyID, p);

            // envoyer le message de room joined au joueur (azy j'ai plus besoin de parler)
            struct lobby_in_info info;
            info.id = lobby.GetID();
            info.name = lobby.GetName();
            info.nbPlayers = lobby.GetNbPlayers();
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                info.id_player.push_back(id);
            }
            AddMessageToPlayer(GameEvents::S_ROOM_JOINED, client->GetID(), info);

            _clientStates[client] = ClientState::IN_LOBBY;
            _toGameMessages.push({GameEvents::S_ROOM_JOINED, client->GetID(), msg});
            return;
        }
    }
    AddMessageToPlayer(GameEvents::S_ROOM_NOT_JOINED, client->GetID(), NULL);
}

void Server::OnClientJoinRandomLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN) {
        AddMessageToPlayer(GameEvents::ASK_LOG, client->GetID(), NULL);
        return;
    }
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.AddPlayer(client)) {
            // envoyer le message de player joined a tous les joueurs du lobby (ca aussi c vignesh MOUROUGANANDAME QUI A
            // ECRIT LE COMMENTAIRE BORIS JE VAIS TE HAGAR)
            struct player p;
            p.id = client->GetID();
            p.username = _clientUsernames[client];
            AddMessageToLobby(GameEvents::S_PLAYER_JOINED, lobby.GetID(), p);

            // envoyer le message de room joined au joueur (azy j'ai plus besoin de parler)
            struct lobby_in_info info;
            info.id = lobby.GetID();
            info.name = lobby.GetName();
            info.nbPlayers = lobby.GetNbPlayers();
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                info.id_player.push_back(id);
            }
            AddMessageToPlayer(GameEvents::S_ROOM_JOINED, client->GetID(), info);

            _clientStates[client] = ClientState::IN_LOBBY;
            message<GameEvents> return_msg;
            return_msg << lobby.GetID();
            _toGameMessages.push({GameEvents::S_ROOM_JOINED, client->GetID(), return_msg});
            return;
        }
    }
    AddMessageToPlayer(GameEvents::S_ROOM_NOT_JOINED, client->GetID(), NULL);
}
void Server::OnClientLeaveLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::IN_LOBBY)
        return;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        auto mapPlayers = lobby.getLobbyPlayers();
        uint32_t lobbyID;
        if (lobby.HasPlayer(client->GetID())) {
            _clientStates[client] = ClientState::LOGGED_IN;
            lobbyID = lobby.GetID();
            AddMessageToPlayer(GameEvents::S_ROOM_LEAVE, client->GetID(), NULL);
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
            AddMessageToLobby(GameEvents::S_PLAYER_LEAVE, lobbyID, client->GetID());
            message<GameEvents> return_msg;
            return_msg << lobbyID;
            _toGameMessages.push({GameEvents::S_PLAYER_LEAVE, client->GetID(), return_msg});
            break;
        }
    }
}

void Server::OnClientNewLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
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
    AddMessageToLobby(GameEvents::S_NEW_HOST, newLobbyID, client->GetID());
    _clientStates[client] = ClientState::IN_LOBBY;
}

void Server::onClientStartGame(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::READY)
        return;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.HasPlayer(client->GetID())) {
            lobby.SetState(Lobby<GameEvents>::State::IN_GAME);
            if (lobby.getOwner() != client) {
                AddMessageToPlayer(GameEvents::S_GAME_START_KO, client->GetID(), NULL);
                return;
            }
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                if (_clientStates[connection] != ClientState::READY) {
                    AddMessageToPlayer(GameEvents::S_GAME_START_KO, client->GetID(), NULL);
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

void Server::onClientReadyUp(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::IN_LOBBY)
        return;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.HasPlayer(client->GetID())) {
            _clientStates[client] = ClientState::READY;
            AddMessageToLobby(GameEvents::S_READY_RETURN, lobby.GetID(), client->GetID());
            break;
        }
    }
}

void Server::onClientUnready(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::READY)
        return;
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.HasPlayer(client->GetID())) {
            _clientStates[client] = ClientState::IN_LOBBY;
            AddMessageToLobby(GameEvents::S_CANCEL_READY_BROADCAST, lobby.GetID(), client->GetID());
            break;
        }
    }
}
