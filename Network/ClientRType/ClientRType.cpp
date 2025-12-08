#include "ClientRType.hpp"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>

void ClientRType::PingServer() {
    AddMessageToServer(RTypeEvents::C_PING_SERVER, 0, NULL);
}

coming_message ClientRType::ReadIncomingMessage() {
    coming_message c_message;
    if (!Incoming().empty()) {
        auto msg = Incoming().pop_front();
        switch (msg.msg.header.id) {
            case RTypeEvents::S_REGISTER_OK:
            case RTypeEvents::S_REGISTER_KO:
            case RTypeEvents::S_LOGIN_OK:
            case RTypeEvents::S_LOGIN_KO:
                msg.msg >> msg.msg.body;
                break;
            default:
                break;
        };

        if (msg.msg.header.id == RTypeEvents::S_REGISTER_OK || msg.msg.header.id == RTypeEvents::S_LOGIN_OK) {
            auto now = std::chrono::system_clock::now();
            auto expiration_time = now + std::chrono::hours(10 * 24);
            auto expiration_timestamp = std::chrono::system_clock::to_time_t(expiration_time);

            connection_server_return response;
            auto tempMsg = msg.msg;
            tempMsg >> response;

            std::string filename = ".secretoken";
            std::ofstream file(filename);
            if (file.is_open()) {
                file << response.token;
                file.close();
            }
            std::ofstream expiration_file(filename + ".expire");
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