#include "Client.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <string>

#include "../Network.hpp"

void network::Client::PingServer() {
    AddMessageToServer(GameEvents::C_PING_SERVER, 0, 0);
}

void network::Client::LoginServerToken() {
    std::ifstream file(TOKEN_FILENAME);
    std::string token;
    if (file.good()) {
        std::getline(file, token);
        file.close();
        char tokenC[32] = {0};
        std::strncpy(tokenC, token.c_str(), 31);
        AddMessageToServer(GameEvents::C_LOGIN_TOKEN, 0, tokenC);
    }
}

void network::Client::LoginServer(std::string username, std::string password) {
    connection_info info;
    std::strncpy(info.username, username.c_str(), 32);
    std::strncpy(info.password, password.c_str(), 32);

    AddMessageToServer(GameEvents::C_LOGIN, 0, info);
}

void network::Client::LoginAnonymous() {
    AddMessageToServer(GameEvents::C_LOGIN_ANONYMOUS, 0);
}

void network::Client::RegisterServer(std::string username, std::string password) {
    connection_info info;
    std::strncpy(info.username, username.c_str(), 32);
    std::strncpy(info.password, password.c_str(), 32);

    AddMessageToServer(GameEvents::C_REGISTER, 0, info);
}

network::coming_message network::Client::ReadIncomingMessage() {
    while (!Incoming().empty()) {
        auto msg = Incoming().pop_front();

        if (msg.msg.header.id == GameEvents::S_REGISTER_OK || msg.msg.header.id == GameEvents::S_LOGIN_OK) {
            auto now = std::chrono::system_clock::now();
            auto expiration_time = now + std::chrono::hours(10 * 24);
            auto expiration_timestamp = std::chrono::system_clock::to_time_t(expiration_time);

            char token[32] = {0};
            msg.msg >> token;

            std::ofstream file(TOKEN_FILENAME);
            if (file.is_open()) {
                file << token;
                file.close();
            }
            std::ofstream expiration_file(static_cast<std::string>(TOKEN_FILENAME) + ".expire");
            expiration_file << expiration_timestamp;
        } else if (msg.msg.header.id == GameEvents::S_SEND_ID) {
            auto temp_msg = msg.msg;
            temp_msg >> _id;
            std::cout << "[CLIENT] ID Received: " << _id << "\n";

            coming_message comingMsg;
            comingMsg.id = msg.msg.header.id;
            comingMsg.msg = msg.msg;
            return comingMsg;
        } else if (msg.msg.header.id == GameEvents::S_CONFIRM_UDP) {
            AddMessageToServer(GameEvents::C_CONFIRM_UDP, 0, 0);
            return ReadIncomingMessage();
        }
        coming_message comingMsg;
        comingMsg.id = msg.msg.header.id;
        comingMsg.msg = msg.msg;
        return comingMsg;
    }
    return coming_message{};
};