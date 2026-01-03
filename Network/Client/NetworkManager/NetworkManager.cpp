#include "NetworkManager.hpp"

using enum network::GameEvents;

NetworkManager::NetworkManager() {
    initializeValidClientEvents();
    initializeTcpEvents();
    initializeUdpEvents();
    initializePayloadConstraints();
}

void NetworkManager::initializeValidClientEvents() {
    _validClientEvents = {C_PING_SERVER, C_REGISTER,    C_LOGIN,      C_LOGIN_TOKEN,        C_DISCONNECT,
                          C_CONFIRM_UDP, C_LIST_ROOMS,  C_JOIN_ROOM,  C_JOINT_RANDOM_LOBBY, C_ROOM_LEAVE,
                          C_NEW_LOBBY,   C_READY,       C_GAME_START, C_CANCEL_READY,       C_INPUT,
                          C_TEAM_CHAT,   C_VOICE_PACKET};
}

void NetworkManager::initializeTcpEvents() {
    _tcpEvents = {C_PING_SERVER, C_REGISTER,  C_LOGIN, C_LOGIN_TOKEN, C_DISCONNECT,   C_LIST_ROOMS, C_JOIN_ROOM,
                  C_ROOM_LEAVE,  C_NEW_LOBBY, C_READY, C_GAME_START,  C_CANCEL_READY, C_TEAM_CHAT};
}

void NetworkManager::initializeUdpEvents() {
    _udpEvents = {C_INPUT, C_CONFIRM_UDP, C_VOICE_PACKET};
}

void NetworkManager::initializePayloadConstraints() {
    _payloadConstraints[C_REGISTER] = {sizeof(network::connection_info), sizeof(network::connection_info)};
    _payloadConstraints[C_LOGIN] = {sizeof(network::connection_info), sizeof(network::connection_info)};
    _payloadConstraints[C_LOGIN_TOKEN] = {sizeof(uint32_t) + 1, 256};
    _payloadConstraints[C_JOIN_ROOM] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_ROOM_LEAVE] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_NEW_LOBBY] = {sizeof(uint32_t) + 1, 128};
    _payloadConstraints[C_READY] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_CANCEL_READY] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_INPUT] = {sizeof(bool) + sizeof(uint32_t) + 1, 256};
    _payloadConstraints[C_CONFIRM_UDP] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_PING_SERVER] = {0, 64};
    _payloadConstraints[C_DISCONNECT] = {0, 0};
    _payloadConstraints[C_LIST_ROOMS] = {0, 0};
    _payloadConstraints[C_GAME_START] = {0, sizeof(uint32_t)};
    _payloadConstraints[C_TEAM_CHAT] = {sizeof(uint32_t) + 1, 1024};
    _payloadConstraints[C_VOICE_PACKET] = {1, 8192};
}

bool NetworkManager::isValidClientEvent(network::GameEvents event) const {
    return _validClientEvents.contains(event);
}

bool NetworkManager::isTcpEvent(network::GameEvents event) const {
    return _tcpEvents.contains(event);
}

bool NetworkManager::isUdpEvent(network::GameEvents event) const {
    return _udpEvents.contains(event);
}

std::optional<size_t> NetworkManager::getMinPayloadSize(network::GameEvents event) const {
    if (auto it = _payloadConstraints.find(event); it != _payloadConstraints.end()) {
        return it->second.first;
    }
    return std::nullopt;
}

std::optional<size_t> NetworkManager::getMaxPayloadSize(network::GameEvents event) const {
    if (auto it = _payloadConstraints.find(event); it != _payloadConstraints.end()) {
        return it->second.second;
    }
    return std::nullopt;
}
