#include "Server.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

using namespace network;

void Server::OnMessage(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents>& msg) {
    auto validation = _networkManager.validateClientPacket(msg.header.id, msg);
    if (!validation.isValid()) {
        return;
    }

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
        case GameEvents::C_LOGIN_ANONYMOUS:
            OnClientLoginAnonymous(client, msg);
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
            // Also forward to game engine so it can send the full game state
            _toGameMessages.push({msg.header.id, client->GetID(), msg});
            break;
        case GameEvents::C_TEAM_CHAT:
            onClientSendText(client, msg);
            break;
        case GameEvents::C_VOICE_PACKET:
            onClientVoicePacket(client, msg);
            break;
        case GameEvents::C_READY:
            onClientReadyUp(client, msg);
            break;
        case GameEvents::C_CANCEL_READY:
            onClientUnready(client, msg);
            break;
        case GameEvents::C_GAME_START:
            onClientStartGame(client, msg);
            break;
        default:
            // Always respect the connection ID, not what the client claims in the header
            _toGameMessages.push({msg.header.id, client->GetID(), msg});
            break;
    }
}

void Server::OnClientDisconnect(std::shared_ptr<Connection<GameEvents>> client) {
    uint32_t clientId = client->GetID();

    // Check if client is in our state map (avoid double processing)
    if (_clientStates.find(client) == _clientStates.end()) {
        return;
    }

    // Remove from state map first to prevent re-entry
    _clientStates.erase(client);

    // Remove player from lobby if they were in one
    uint32_t lobbyToDelete = 0;
    bool shouldBroadcast = false;

    for (auto& lobby : _lobbys) {
        if (lobby.HasPlayer(clientId)) {
            auto mapPlayers = lobby.getLobbyPlayers();
            uint32_t lobbyID = lobby.GetID();

            // If this player was the owner, transfer ownership or mark for deletion
            if (mapPlayers.size() == 1) {
                // Last player - mark lobby for deletion
                lobbyToDelete = lobbyID;
            } else if (mapPlayers[clientId] == lobby.getOwner()) {
                // Transfer ownership
                for (auto& [id, connection] : mapPlayers) {
                    if (id != clientId) {
                        lobby.setOwner(connection);
                        AddMessageToLobby(GameEvents::S_NEW_HOST, lobbyID, id);
                        break;
                    }
                }
                lobby.RemovePlayer(clientId);
                AddMessageToLobby(GameEvents::S_PLAYER_LEAVE, lobbyID, clientId);
            } else {
                lobby.RemovePlayer(clientId);
                AddMessageToLobby(GameEvents::S_PLAYER_LEAVE, lobbyID, clientId);
            }
            shouldBroadcast = true;
            break;
        }
    }

    // Delete lobby outside the loop
    if (lobbyToDelete > 0) {
        _lobbys.erase(
            std::remove_if(_lobbys.begin(), _lobbys.end(),
                           [lobbyToDelete](const Lobby<GameEvents>& l) { return l.GetID() == lobbyToDelete; }),
            _lobbys.end());
    }

    if (shouldBroadcast) {
        BroadcastLobbyList();
    }

    message<GameEvents> msg;
    msg << clientId;
    _toGameMessages.push({GameEvents::C_DISCONNECT, clientId, msg});
}

bool Server::OnClientConnect(std::shared_ptr<Connection<GameEvents>> client) {
    if (_deqConnections.size() >= _maxConnections)
        return false;
    client->SetTimeout(0);

    AddMessageToPlayer(GameEvents::S_SEND_ID, client->GetID(), client->GetID());
    network::message<GameEvents> msg;
    msg << client->GetID();
    AddMessageToPlayer(GameEvents::S_CONFIRM_UDP, client->GetID(), msg);

    msg.header.user_id = client->GetID();
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

    std::string tokenStr = "";
    const std::string charset = ALPHA_NUMERIC;
    srand(time(nullptr) + client->GetID());
    for (int i = 0; i < 10; i++) {
        tokenStr += charset[rand() % charset.length()];
    }
    _database.SaveToken(_database.LoginUser(info.username, info.password), tokenStr);

    _clientUsernames[client] = info.username;
    _clientStates[client] = ClientState::LOGGED_IN;

    char token[32] = {0};
    std::strncpy(token, tokenStr.c_str(), 31);
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

    std::string tokenStr = _database.GetTokenById(userID);
    char token[32] = {0};
    std::strncpy(token, tokenStr.c_str(), 31);

    _clientUsernames[client] = info.username;
    _clientStates[client] = ClientState::LOGGED_IN;
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), token);
}

void Server::OnClientLoginToken(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    char token[32];
    msg >> token;
    std::string tokenStr(token);

    if (_clientStates[client] != ClientState::CONNECTED) {
        AddMessageToPlayer(GameEvents::ASK_UDP, client->GetID(), NULL);
        return;
    }
    int userID = _database.GetUserByToken(tokenStr);
    if (userID == -1) {
        AddMessageToPlayer(GameEvents::S_INVALID_TOKEN, client->GetID(), NULL);
        return;
    }

    _clientUsernames[client] = _database.GetNameById(userID);
    _clientStates[client] = ClientState::LOGGED_IN;
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), NULL);
}

void Server::OnClientLoginAnonymous(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::CONNECTED) {
        AddMessageToPlayer(GameEvents::ASK_UDP, client->GetID(), NULL);
        return;
    }

    std::string guestName = "Guest_" + std::to_string(client->GetID());
    _clientUsernames[client] = guestName;
    _clientStates[client] = ClientState::LOGGED_IN;

    // Send empty token
    char token[32] = {0};
    AddMessageToPlayer(GameEvents::S_LOGIN_OK, client->GetID(), token);
}

void Server::OnClientListLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN) {
        AddMessageToPlayer(GameEvents::ASK_LOG, client->GetID(), NULL);
        return;
    }
    try {
        network::message<GameEvents> responseMsg;
        responseMsg.header.id = GameEvents::S_ROOMS_LIST;
        responseMsg.header.user_id = client->GetID();

        uint32_t nb_lobbys = _lobbys.size();

        // Push lobbies directly
        for (Lobby<GameEvents>& lobby : _lobbys) {
            lobby_info info;
            info.id = lobby.GetID();
            std::strncpy(info.name, lobby.GetName().c_str(), 31);  // Safe copy
            info.name[31] = '\0';
            info.nbConnectedPlayers = lobby.GetNbPlayers();
            info.maxPlayers = lobby.GetMaxPlayers();
            info.state = (uint32_t)lobby.GetState();
            responseMsg << info;
        }
        // Push count last (so it is popped first)
        responseMsg << nb_lobbys;

        client->Send(responseMsg);
    } catch (const std::exception& e) {
    } catch (...) {}
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
            std::strncpy(p.username, _clientUsernames[client].c_str(), 32);
            AddMessageToLobby(GameEvents::S_PLAYER_JOINED, lobbyID, p);

            // envoyer le message de room joined au joueur (azy j'ai plus besoin de parler)
            struct lobby_in_info info;
            info.id = lobby.GetID();
            std::strncpy(info.name, lobby.GetName().c_str(), 32);
            // Do NOT populate players from info.id_player anymore.
            // Server will send S_PLAYER_JOINED for each existing player (including us, if it broadcasts).
            // Actually, server broadcasts new player to everyone (including us).
            // And we just added logic to server to send existing players to us.
            // So we rely entirely on S_PLAYER_JOINED messages.
            info.nbPlayers = lobby.GetNbPlayers();
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                // Send existing player info to the new joiner
                if (connection != client) {
                    struct player existingPlayer;
                    existingPlayer.id = id;
                    std::strncpy(existingPlayer.username, _clientUsernames[connection].c_str(), 32);
                    AddMessageToPlayer(GameEvents::S_PLAYER_JOINED, client->GetID(), existingPlayer);
                }
            }
            AddMessageToPlayer(GameEvents::S_ROOM_JOINED, client->GetID(), info);

            _clientStates[client] = ClientState::IN_LOBBY;
            BroadcastLobbyList();
            // Create a new message with lobby info for the game engine
            message<GameEvents> gameMsg;
            gameMsg.header.id = GameEvents::S_ROOM_JOINED;
            gameMsg.header.user_id = client->GetID();
            gameMsg << info;
            _toGameMessages.push({GameEvents::S_ROOM_JOINED, client->GetID(), gameMsg});
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
            std::strncpy(p.username, _clientUsernames[client].c_str(), 32);
            AddMessageToLobby(GameEvents::S_PLAYER_JOINED, lobby.GetID(), p);

            // envoyer le message de room joined au joueur (azy j'ai plus besoin de parler)
            struct lobby_in_info info;
            info.id = lobby.GetID();
            std::strncpy(info.name, lobby.GetName().c_str(), 32);
            info.nbPlayers = lobby.GetNbPlayers();
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                info.id_player.push_back(id);
            }
            if (lobby.getOwner())
                info.hostId = lobby.getOwner()->GetID();
            else
                info.hostId = 0;

            AddMessageToPlayer(GameEvents::S_ROOM_JOINED, client->GetID(), info);

            _clientStates[client] = ClientState::IN_LOBBY;
            BroadcastLobbyList();  // Broadcast updated state
            // Create a new message with lobby info for the game engine
            message<GameEvents> return_msg;
            return_msg.header.id = GameEvents::S_ROOM_JOINED;
            return_msg.header.user_id = client->GetID();
            return_msg << info;
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
                    // Last player leaving - delete the lobby
                    _lobbys.erase(
                        std::remove_if(_lobbys.begin(), _lobbys.end(),
                                       [lobbyID](const Lobby<GameEvents>& lobby) { return lobby.GetID() == lobbyID; }),
                        _lobbys.end());
                    BroadcastLobbyList();
                    return;
                }
            }
            lobby.RemovePlayer(client->GetID());
            AddMessageToLobby(GameEvents::S_PLAYER_LEAVE, lobbyID, client->GetID());
            BroadcastLobbyList();
            message<GameEvents> return_msg;
            return_msg.header.user_id = client->GetID();
            return_msg << lobbyID;
            _toGameMessages.push({GameEvents::S_PLAYER_LEAVE, client->GetID(), return_msg});
            break;
        }
    }
}

void Server::OnClientNewLobby(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::LOGGED_IN) {
        return;
    }

    char name[32] = {0};
    try {
        msg >> name;
    } catch (const std::exception& e) {
        return;
    }

    std::string lobbyName = name;

    // Check if player already in lobby? (Optional)

    Lobby<GameEvents> newLobby(nLobbyIDCounter++, lobbyName);
    newLobby.AddPlayer(client);
    _lobbys.push_back(newLobby);
    _clientStates[client] = ClientState::IN_LOBBY;

    // Send confirmation
    char lobbyNameBuff[32] = {0};
    std::strncpy(lobbyNameBuff, lobbyName.c_str(), 31);
    AddMessageToPlayer(GameEvents::S_CONFIRM_NEW_LOBBY, client->GetID(), lobbyNameBuff);

    // Send S_PLAYER_JOINED to host so they appear in their own player list
    struct player hostPlayer;
    hostPlayer.id = client->GetID();
    std::strncpy(hostPlayer.username, _clientUsernames[client].c_str(), 32);
    AddMessageToPlayer(GameEvents::S_PLAYER_JOINED, client->GetID(), hostPlayer);

    // Also notify game engine about the new lobby
    network::message<GameEvents> lobbyMsg;
    lobbyMsg.header.id = GameEvents::S_ROOM_JOINED;
    lobbyMsg.header.user_id = client->GetID();
    network::lobby_in_info info;
    info.id = newLobby.GetID();
    std::strncpy(info.name, lobbyName.c_str(), 32);
    info.nbPlayers = 1;
    info.hostId = client->GetID();
    lobbyMsg << info;
    _toGameMessages.push({GameEvents::S_ROOM_JOINED, client->GetID(), lobbyMsg});

    BroadcastLobbyList();
}

void Server::BroadcastLobbyList() {
    network::message<GameEvents> listMsg;
    listMsg.header.id = GameEvents::S_ROOMS_LIST;

    uint32_t nb_lobbys = _lobbys.size();
    for (Lobby<GameEvents>& lobby : _lobbys) {
        lobby_info info;
        info.id = lobby.GetID();
        std::strncpy(info.name, lobby.GetName().c_str(), 31);
        info.name[31] = '\0';
        info.nbConnectedPlayers = lobby.GetNbPlayers();
        info.maxPlayers = lobby.GetMaxPlayers();
        info.state = (uint32_t)lobby.GetState();
        listMsg << info;
    }
    listMsg << nb_lobbys;

    for (auto& [client, state] : _clientStates) {
        if (client && client->IsConnected()) {
            MessageClient(client, listMsg);
        }
    }
}

void Server::onClientStartGame(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::READY) {
        return;
    }
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.HasPlayer(client->GetID())) {
            if (lobby.getOwner() != client) {
                AddMessageToPlayer(GameEvents::S_GAME_START_KO, client->GetID(), NULL);
                return;
            }
            // Check all players ready
            for (auto& [id, connection] : lobby.getLobbyPlayers()) {
                if (_clientStates[connection] != ClientState::READY) {
                    AddMessageToPlayer(GameEvents::S_GAME_START_KO, client->GetID(), NULL);
                    return;
                }
            }

            // All checks passed. Set state and broadcast.
            lobby.SetState(Lobby<GameEvents>::State::IN_GAME);

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

            _toGameMessages.push({GameEvents::C_READY, client->GetID(), msg});
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

            _toGameMessages.push({GameEvents::C_CANCEL_READY, client->GetID(), msg});
            break;
        }
    }
}

void Server::onClientSendText(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents> msg) {
    if (_clientStates[client] != ClientState::IN_LOBBY && _clientStates[client] != ClientState::READY)
        return;

    // Extract message text from the incoming message
    char messageText[256] = {0};
    if (msg.body.size() >= 256) {
        std::memcpy(messageText, msg.body.data() + msg.body.size() - 256, 256);
        msg.body.resize(msg.body.size() - 256);
    } else if (msg.body.size() > 0) {
        std::memcpy(messageText, msg.body.data(), msg.body.size());
        msg.body.clear();
    }

    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.HasPlayer(client->GetID())) {
            if (lobby.GetState() == Lobby<GameEvents>::State::IN_GAME)
                return;

            // Create chat_message struct with sender info and message
            chat_message chatMsg;
            chatMsg.sender_id = client->GetID();
            std::strncpy(chatMsg.sender_name, _clientUsernames[client].c_str(), 31);
            chatMsg.sender_name[31] = '\0';
            std::strncpy(chatMsg.message, messageText, 255);
            chatMsg.message[255] = '\0';

            AddMessageToLobby(GameEvents::S_TEAM_CHAT, lobby.GetID(), chatMsg);
            break;
        }
    }
}

void Server::onClientVoicePacket(std::shared_ptr<Connection<GameEvents>> client, message<GameEvents>& msg) {
    // Find the lobby the client is in
    for (Lobby<GameEvents>& lobby : _lobbys) {
        if (lobby.HasPlayer(client->GetID())) {
            // Get the voice packet data
            if (msg.body.size() < sizeof(voice_packet)) {
                return;
            }

            voice_packet voiceData;
            std::memcpy(&voiceData, msg.body.data(), sizeof(voice_packet));

            // Set the sender ID to the client's actual ID
            voiceData.sender_id = client->GetID();

            // Relay to all other players in the lobby
            static uint32_t relayCount = 0;
            int relayedTo = 0;
            for (auto& otherClient : _deqConnections) {
                if (otherClient && otherClient->GetID() != client->GetID() && lobby.HasPlayer(otherClient->GetID())) {
                    message<GameEvents> relayMsg;
                    relayMsg.header.id = GameEvents::S_VOICE_RELAY;
                    relayMsg.body.resize(sizeof(voice_packet));
                    std::memcpy(relayMsg.body.data(), &voiceData, sizeof(voice_packet));
                    relayMsg.header.size = static_cast<uint32_t>(relayMsg.size());
                    MessageClient(otherClient, relayMsg);
                    relayedTo++;
                }
            }

            relayCount++;
            break;
        }
    }
}