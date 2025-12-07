#include "ClientRType.hpp"
#include <fstream>
#include <string>

void ClientRType::PingServer() {
    network::message<RTypeEvents> msg;
    msg.header.id = RTypeEvents::ServerPing;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

    msg << timeNow;
    Send(msg);
}

coming_message ClientRType::ReadIncomingMessage() {
    return ([&]() {
        if (!Incoming().empty()) {
            auto msg = Incoming().pop_front();
            if (msg.msg.header.id == RTypeEvents::ServerAccept) {
                char token[64];
                auto tempMsg = msg.msg;
                tempMsg >> token;
                std::ofstream file(".secret" + std::to_string(msg.msg.header.user_id));
                if (file.is_open()) {
                    file << token;
                    file.close();
                }
            }
            coming_message comingMsg;
            comingMsg.id = msg.msg.header.id;
            msg.msg >> comingMsg.msg.body;
            return comingMsg;
        }
        return coming_message{};
    }());
};