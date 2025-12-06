#include "ClientRType.hpp"

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
            coming_message comingMsg;
            comingMsg.id = msg.msg.header.id;
            msg.msg >> comingMsg.msg.body;
            return comingMsg;
        }
        return coming_message{};
    }());
};