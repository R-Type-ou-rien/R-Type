#include "ServerNetworkManager.hpp"
#include "../../Network.hpp"  // For sizeof constraints on structs

using enum network::GameEvents;

ServerNetworkManager::ServerNetworkManager() {
    initializeValidClientEvents();
    initializeTcpEvents();
    initializeUdpEvents();
    initializePayloadConstraints();
}

void ServerNetworkManager::initializeValidClientEvents() {
    // Events that the server expects to RECEIVE from clients (C_...)
    _validClientEvents = {C_PING_SERVER, C_REGISTER,    C_LOGIN,      C_LOGIN_TOKEN,        C_DISCONNECT,
                          C_CONFIRM_UDP, C_LIST_ROOMS,  C_JOIN_ROOM,  C_JOINT_RANDOM_LOBBY, C_ROOM_LEAVE,
                          C_NEW_LOBBY,   C_READY,       C_GAME_START, C_CANCEL_READY,       C_INPUT,
                          C_TEAM_CHAT,   C_VOICE_PACKET};
}

void ServerNetworkManager::initializeTcpEvents() {
    // Events that the server SENDS via TCP (S_...)
    _tcpEvents = {S_REGISTER_OK, S_LOGIN_OK, S_LOGIN_KO, S_ROOMS_LIST, S_ROOM_JOINED, S_ROOM_NOT_JOINED,
                  S_CONFIRM_NEW_LOBBY, S_PLAYER_JOINED,
                  // S_ROOM_INFO, // Doesn't seem to exist in Network.hpp
                  S_ROOM_LEAVE, S_READY_RETURN, S_CANCEL_READY_BROADCAST, S_GAME_START, S_SEND_ID, S_CONFIRM_UDP,
                  S_TEAM_CHAT, S_RETURN_TO_LOBBY, S_GAME_OVER, S_PLAYER_DEATH};
}

void ServerNetworkManager::initializeUdpEvents() {
    // Events that the server SENDS via UDP (S_...)
    _udpEvents = {S_SNAPSHOT, S_VOICE_RELAY};
}

void ServerNetworkManager::initializePayloadConstraints() {
    // Constraints for C_ events received from clients
    _payloadConstraints[C_REGISTER] = {sizeof(network::connection_info), sizeof(network::connection_info)};
    _payloadConstraints[C_LOGIN] = {sizeof(network::connection_info), sizeof(network::connection_info)};
    _payloadConstraints[C_LOGIN_TOKEN] = {sizeof(uint32_t) + 1, 256};  // Token length variable but max 256 safe guess
    _payloadConstraints[C_JOIN_ROOM] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_ROOM_LEAVE] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_NEW_LOBBY] = {sizeof(uint32_t) + 1, 128};  // Lobby name
    _payloadConstraints[C_READY] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_CANCEL_READY] = {sizeof(uint32_t), sizeof(uint32_t)};
    _payloadConstraints[C_INPUT] = {sizeof(bool) + sizeof(uint32_t) + 1, 256};
    _payloadConstraints[C_CONFIRM_UDP] = {sizeof(uint32_t), sizeof(uint32_t)};  // Usually just sends ID or 0
    _payloadConstraints[C_PING_SERVER] = {0, 64};
    _payloadConstraints[C_DISCONNECT] = {0, 0};
    _payloadConstraints[C_LIST_ROOMS] = {0, 0};
    _payloadConstraints[C_GAME_START] = {0, sizeof(uint32_t)};
    _payloadConstraints[C_TEAM_CHAT] = {sizeof(uint32_t) + 1, 1024};
    _payloadConstraints[C_VOICE_PACKET] = {1, 8192};
}

bool ServerNetworkManager::isValidClientEvent(network::GameEvents event) const {
    return _validClientEvents.contains(event);
}

bool ServerNetworkManager::isTcpEvent(network::GameEvents event) const {
    return _tcpEvents.contains(event);
}

bool ServerNetworkManager::isUdpEvent(network::GameEvents event) const {
    return _udpEvents.contains(event);
}
