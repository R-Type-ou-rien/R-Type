#pragma once
#include <cstdint>
#include <vector>

#include "NetworkInterface/message.hpp"

enum class RTypeEvents : uint32_t {
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

struct coming_message {
    RTypeEvents id;
    network::message<RTypeEvents> msg;
};