#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include "InputAction.hpp"
#include "InputState.hpp"
#include "../../../Network/Network.hpp"

enum ResourceAction { LOAD_RES, DELETE_RES };

enum PacketResourceType {
    TEXTURE,
    SOUND,
    MUSIC,
};

struct NetworkIdentity {
    static constexpr auto name = "NetworkIdentityComponent";
    uint32_t guid;
    uint32_t ownerId;
};

struct ComponentPacket {
    static constexpr auto name = "ComponentPacket";
    uint32_t entity_guid;
    uint32_t component_type;
    uint32_t owner_id;  // Added owner_id
    std::vector<uint8_t> data;
};

struct ActionPacket {
    static constexpr auto name = "ActionPacket";
    Action action_name;
    ActionState action_state;
};

struct ResourcePacket {
    static constexpr auto name = "ResourcePacket";
    std::vector<std::string> resources_source;
    ResourceAction action;
    PacketResourceType type;
};

namespace network {

static constexpr uint32_t MAX_COMPONENT_PACKET_DATA_SIZE = 256u * 1024u;  // 256 KiB
static constexpr uint32_t MAX_ACTION_NAME_SIZE = 256u;

inline message<GameEvents>& operator<<(message<GameEvents>& msg, const ComponentPacket& packet) {
    for (const auto& byte : packet.data) {
        msg << byte;
    }
    uint32_t size = static_cast<uint32_t>(packet.data.size());
    msg << size;
    msg << packet.owner_id;  // Serialize owner_id
    msg << packet.component_type;
    msg << packet.entity_guid;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, ComponentPacket& packet) {
    msg >> packet.entity_guid;
    msg >> packet.component_type;
    msg >> packet.owner_id;  // Deserialize owner_id
    uint32_t size = 0;
    msg >> size;

    const uint32_t available = static_cast<uint32_t>(msg.body.size());
    if (size > available || size > MAX_COMPONENT_PACKET_DATA_SIZE) {
        packet.data.clear();
        msg.body.clear();
        msg.header.size = 0;
        return msg;
    }

    packet.data.resize(size);
    for (uint32_t idx = 0; idx < size; ++idx) {
        msg >> packet.data[size - 1 - idx];
    }
    return msg;
}

inline message<GameEvents>& operator<<(message<GameEvents>& msg, const ActionPacket& packet) {
    for (const auto& c : packet.action_name) {
        msg << c;
    }
    uint32_t size = static_cast<uint32_t>(packet.action_name.size());
    msg << size;
    msg << packet.action_state.pressed;
    msg << packet.action_state.justPressed;
    msg << packet.action_state.justReleased;
    msg << packet.action_state.holdTime;
    msg << packet.action_state.lastReleaseHoldTime;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, ActionPacket& packet) {
    msg >> packet.action_state.lastReleaseHoldTime;
    msg >> packet.action_state.holdTime;
    msg >> packet.action_state.justReleased;
    msg >> packet.action_state.justPressed;
    msg >> packet.action_state.pressed;
    uint32_t size = 0;
    msg >> size;

    const uint32_t available = static_cast<uint32_t>(msg.body.size());
    if (size > available || size > MAX_ACTION_NAME_SIZE) {
        packet.action_name.clear();
        msg.body.clear();
        msg.header.size = 0;
        return msg;
    }

    packet.action_name.resize(size);
    for (uint32_t idx = 0; idx < size; ++idx) {
        msg >> packet.action_name[size - 1 - idx];
    }
    return msg;
}

}  // namespace network
