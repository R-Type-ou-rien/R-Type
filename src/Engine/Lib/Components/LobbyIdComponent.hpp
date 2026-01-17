#pragma once

#include <cstdint>

/**
 * @brief Component to identify which lobby an entity belongs to.
 *
 * This is used to isolate entities between different game lobbies on the server.
 * Entities with different lobby_id values should not interact with each other
 * (no collisions, no damage, etc.)
 */
struct LobbyIdComponent {
    static constexpr auto name = "LobbyIdComponent";
    uint32_t lobby_id = 0;
};
