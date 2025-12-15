#pragma once

#include <cstdint>
#include <vector>

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

