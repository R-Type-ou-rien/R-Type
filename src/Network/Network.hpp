#pragma once
#include <cstdint>

enum class RTypeEvents : uint32_t {
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

template <typename T>
struct coming_message {
    RTypeEvents id;
    T msg;
};