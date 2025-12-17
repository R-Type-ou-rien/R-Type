#include "Client.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "../Network.hpp"

void Client::PingServer() {
    AddMessageToServer(GameEvents::C_PING_SERVER, 0, NULL);
}

void Client::LoginServerToken() {
    std::ifstream file(TOKEN_FILENAME);
    std::string token;
    if (file.good()) {
        std::getline(file, token);
        file.close();
        AddMessageToServer(GameEvents::C_LOGIN_TOKEN, 0, token);
    }
}

void Client::LoginServer(std::string username, std::string password) {
    struct connection_info info;
    std::strncpy(info.username, username.c_str(), sizeof(info.username));
    std::strncpy(info.password, password.c_str(), sizeof(info.password));

    AddMessageToServer(GameEvents::C_LOGIN, 0, info);
}

void Client::RegisterServer(std::string username, std::string password) {
    struct connection_info info;
    std::strncpy(info.username, username.c_str(), sizeof(info.username));
    std::strncpy(info.password, password.c_str(), sizeof(info.password));

    AddMessageToServer(GameEvents::C_REGISTER, 0, info);
}

coming_message Client::ReadIncomingMessage() {
    coming_message c_message;
    if (!Incoming().empty()) {
        auto msg = Incoming().pop_front();

        coming_message comingMsg;
        comingMsg.id = msg.msg.header.id;
        comingMsg.msg = msg.msg;
        // msg.msg >> comingMsg.msg.body;
        return comingMsg;
    }
    return coming_message{};
};