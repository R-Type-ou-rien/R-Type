#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
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

template <typename T>
network::message<T>& operator<<(network::message<T>& msg, const ComponentPacket& packet) {
    std::cout << "[DEBUG] Serializing Packet: GUID=" << packet.entity_guid << " Type=" << packet.component_type
              << " DataSize=" << packet.data.size() << std::endl;
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

    std::cout << "[DEBUG] Deserializing Packet: GUID=" << packet.entity_guid << " Type=" << packet.component_type
              << " Size=" << size << std::endl;

    packet.data.resize(size);
    for (int i = size - 1; i >= 0; --i) {
        msg >> packet.data[i];
    }
    return msg;
}
