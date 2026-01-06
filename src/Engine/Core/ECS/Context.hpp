#pragma once

#include "ResourceConfig.hpp"
#include "InputConfig.hpp"

#if defined(SERVER_BUILD)
#include "ServerResourceManager.hpp"
#include "NetworkEngine/NetworkEngine.hpp"
struct system_context {
    float dt;
    uint32_t tick;
    ResourceManager<TextureAsset>& texture_manager;
    InputManager& input;
    engine::core::NetworkEngine& network;
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
};
#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif

struct SerializationContext {
    ResourceManager<TextureAsset>& textureManager;
};
