#include "ClientRType.hpp"

void ClientRType::PingServer() {
    network::message<RTypeEvents> msg;
    msg.header.id = RTypeEvents::ServerPing;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

    msg << timeNow;
    Send(msg);
}

void ClientRType::MessageAll() {
    network::message<RTypeEvents> msg;
    msg.header.id = RTypeEvents::MessageAll;
    Send(msg);
}