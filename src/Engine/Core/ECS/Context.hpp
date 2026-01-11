#pragma once

#include <cstdint>
#include "ResourceConfig.hpp"
#include "InputConfig.hpp"

#if defined(SERVER_BUILD)
#include "ServerResourceManager.hpp"
namespace engine::core {
class LobbyManager;
class NetworkEngine;
}  // namespace engine::core

struct system_context {
    float dt;
    uint32_t tick;
    ResourceManager<TextureAsset>& texture_manager;
    InputManager& input;
    engine::core::NetworkEngine& network;
    std::vector<uint32_t> active_clients;
    engine::core::LobbyManager* lobby_manager;
};

#elif defined(CLIENT_BUILD)
#include "ClientResourceManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
struct system_context {
    float dt;
    uint32_t tick;
    ResourceManager<TextureAsset>& texture_manager;
    sf::RenderWindow& window;
    InputManager& input;
    uint32_t player_id;
};
#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif

struct SerializationContext {
    ResourceManager<TextureAsset>& textureManager;
};
