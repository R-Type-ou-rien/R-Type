#include "Client.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <string>

#include "../Network.hpp"

void network::Client::PingServer() {
    AddMessageToServer(GameEvents::C_PING_SERVER, 0, NULL);
}

void network::Client::LoginServerToken() {
    std::ifstream file(TOKEN_FILENAME);
    std::string token;
    if (file.good()) {
        std::getline(file, token);
        file.close();
        AddMessageToServer(GameEvents::C_LOGIN_TOKEN, 0, token);
    }
}

void network::Client::LoginServer(std::string username, std::string password) {
    connection_info info = {username, password};

    AddMessageToServer(GameEvents::C_LOGIN, 0, info);
}

void network::Client::RegisterServer(std::string username, std::string password) {
    connection_info info = {username, password};

    AddMessageToServer(GameEvents::C_REGISTER, 0, info);
}

network::coming_message network::Client::ReadIncomingMessage() {
    coming_message c_message;
    if (!Incoming().empty()) {
        auto msg = Incoming().pop_front();

        if (msg.msg.header.id == GameEvents::S_REGISTER_OK || msg.msg.header.id == GameEvents::S_LOGIN_OK) {
            auto now = std::chrono::system_clock::now();
            auto expiration_time = now + std::chrono::hours(10 * 24);
            auto expiration_timestamp = std::chrono::system_clock::to_time_t(expiration_time);

            std::string token;
            msg.msg >> token;

            std::ofstream file(TOKEN_FILENAME);
            if (file.is_open()) {
                file << token;
                file.close();
            }
            std::ofstream expiration_file(static_cast<std::string>(TOKEN_FILENAME) + ".expire");
            expiration_file << expiration_timestamp;
        }
        coming_message comingMsg;
        comingMsg.id = msg.msg.header.id;
        msg.msg >> comingMsg.msg;
        return comingMsg;
    }
    return coming_message{};
};