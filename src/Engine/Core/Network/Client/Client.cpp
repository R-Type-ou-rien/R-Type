#include "Client.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <string>

#include "../Network.hpp"

void Client::PingServer() {
    AddMessageToServer(RTypeEvents::C_PING_SERVER, 0, NULL);
}

void Client::LoginServerToken() {
    std::ifstream file(TOKEN_FILENAME);
    std::string token;
    if (file.good()) {
        std::getline(file, token);
        file.close();
        AddMessageToServer(RTypeEvents::C_LOGIN_TOKEN, 0, token);
    }
}

void Client::LoginServer(std::string username, std::string password) {
    struct connection_info info = {username, password};

    AddMessageToServer(RTypeEvents::C_LOGIN, 0, info);
}

void Client::RegisterServer(std::string username, std::string password) {
    struct connection_info info = {username, password};

    AddMessageToServer(RTypeEvents::C_REGISTER, 0, info);
}

coming_message Client::ReadIncomingMessage() {
    coming_message c_message;
    if (!Incoming().empty()) {
        auto msg = Incoming().pop_front();

        if (msg.msg.header.id == RTypeEvents::S_REGISTER_OK || msg.msg.header.id == RTypeEvents::S_LOGIN_OK) {
            auto now = std::chrono::system_clock::now();
            auto expiration_time = now + std::chrono::hours(10 * 24);
            auto expiration_timestamp = std::chrono::system_clock::to_time_t(expiration_time);

            connection_server_return response;
            auto tempMsg = msg.msg;
            tempMsg >> response;

            std::ofstream file(TOKEN_FILENAME);
            if (file.is_open()) {
                file << response.token;
                file.close();
            }
            std::ofstream expiration_file(static_cast<std::string>(TOKEN_FILENAME) + ".expire");
            expiration_file << expiration_timestamp;
        } else {
            coming_message comingMsg;
            comingMsg.id = msg.msg.header.id;
            msg.msg >> comingMsg.msg.body;
            return comingMsg;
        }
    }
    return coming_message{};
};