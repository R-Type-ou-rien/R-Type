#include "ServerRType.hpp"
#include <string>
#include <cstring>

void ServerRType::OnMessage(std::shared_ptr<network::Connection<RTypeEvents>> client,
                            network::message<RTypeEvents>& msg) {
    switch (msg.header.id) {
        case RTypeEvents::ServerPing: {
            std::cout << "[" << client->GetID() << "]: Server Ping\n";

            client->Send(msg);
        } break;

        case RTypeEvents::MessageAll: {
            std::cout << "[" << client->GetID() << "]: Message All\n";

            network::message<RTypeEvents> msg;
            msg.header.id = RTypeEvents::ServerMessage;
            msg << client->GetID();
            MessageAllClients(msg, client);

        } break;
        case RTypeEvents::ServerAccept:
        case RTypeEvents::ServerDeny:
        case RTypeEvents::ServerMessage:
            break;
    }
}

void ServerRType::OnClientDisconnect(std::shared_ptr<network::Connection<RTypeEvents>> client) {
    std::cout << "Removing client [" << client->GetID() << "]\n";
}

bool ServerRType::OnClientConnect(std::shared_ptr<network::Connection<RTypeEvents>> client) {
    network::message<RTypeEvents> msg;
    msg.header.id = RTypeEvents::ServerAccept;
    msg.header.user_id = client->GetID();
    char token[64] = {0};
    std::string tokenStr = "SecretToken_" + std::to_string(client->GetID());
    std::strncpy(token, tokenStr.c_str(), 63);
    msg << token;
    client->Send(msg);
    return true;
}
coming_message ServerRType::ReadIncomingMessage() {
    if (_toGameMessages.empty())
        return coming_message{};

    coming_message message = _toGameMessages.front();
    _toGameMessages.pop();
    return message;
}