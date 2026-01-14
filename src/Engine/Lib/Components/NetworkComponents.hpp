#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include "InputAction.hpp"
#include "InputState.hpp"
#include "../../../Network/Network.hpp"

enum ResourceAction { LOAD, DELETE };

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

inline message<GameEvents>& operator<<(message<GameEvents>& msg, const ComponentPacket& packet) {
    for (const auto& byte : packet.data) {
        msg << byte;
    }
    uint32_t size = static_cast<uint32_t>(packet.data.size());
    msg << size;
    msg << packet.component_type;
    msg << packet.entity_guid;
    return msg;
}

inline message<GameEvents>& operator>>(message<GameEvents>& msg, ComponentPacket& packet) {
    msg >> packet.entity_guid;
    msg >> packet.component_type;
    uint32_t size = 0;
    msg >> size;
    packet.data.resize(size);
    for (int i = size - 1; i >= 0; --i) {
        msg >> packet.data[i];
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
    packet.action_name.resize(size);
    for (int i = size - 1; i >= 0; --i) {
        msg >> packet.action_name[i];
    }
    return msg;
}

}  // namespace network