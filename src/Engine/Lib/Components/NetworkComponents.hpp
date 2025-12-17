#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include "InputAction.hpp"
#include "InputState.hpp"
#include "Network/NetworkInterface/message.hpp"

struct NetworkIdentity {
    static constexpr auto name = "Network";
    uint64_t guid;
    uint32_t owner_user_id;
};

struct ComponentPacket {
    static constexpr auto name = "ComponentPacket";
    uint32_t entity_guid;
    uint32_t component_type;
    std::vector<uint8_t> data;
};

struct InputPacket {
    static constexpr auto name = "InputPacket";
    Action action_name;
    ActionState state;
};

template <typename T>
network::message<T>& operator<<(network::message<T>& msg, const ComponentPacket& packet) {
    // std::cout << "[DEBUG] Serializing Packet: GUID=" << packet.entity_guid << " Type=" << packet.component_type
    //           << " DataSize=" << packet.data.size() << std::endl;
    for (const auto& byte : packet.data) {
        msg << byte;
    }
    uint32_t size = static_cast<uint32_t>(packet.data.size());
    msg << size;
    msg << packet.component_type;
    msg << packet.entity_guid;
    return msg;
}

template <typename T>
network::message<T>& operator>>(network::message<T>& msg, ComponentPacket& packet) {
    msg >> packet.entity_guid;
    msg >> packet.component_type;
    uint32_t size = 0;
    msg >> size;

    // std::cout << "[DEBUG] Deserializing Packet: GUID=" << packet.entity_guid << " Type=" << packet.component_type
    //           << " Size=" << size << std::endl;

    packet.data.resize(size);
    for (int i = size - 1; i >= 0; --i) {
        msg >> packet.data[i];
    }
    return msg;
}

template <typename T>
network::message<T>& operator<<(network::message<T>& msg, const InputPacket& packet) {
    for (const auto& c : packet.action_name) {
        msg << c;
    }
    uint32_t size = static_cast<uint32_t>(packet.action_name.size());
    msg << size;
    msg << packet.state.pressed;
    msg << packet.state.justPressed;
    msg << packet.state.justReleased;
    msg << packet.state.holdTime;
    msg << packet.state.lastReleaseHoldTime;
    return msg;
}

template <typename T>
network::message<T>& operator>>(network::message<T>& msg, InputPacket& packet) {
    msg >> packet.state.lastReleaseHoldTime;
    msg >> packet.state.holdTime;
    msg >> packet.state.justReleased;
    msg >> packet.state.justPressed;
    msg >> packet.state.pressed;
    uint32_t size = 0;
    msg >> size;
    packet.action_name.resize(size);
    for (int i = size - 1; i >= 0; --i) {
        msg >> packet.action_name[i];
    }
    return msg;
}
