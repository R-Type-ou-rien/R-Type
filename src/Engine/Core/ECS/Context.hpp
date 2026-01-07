#pragma once

#include "ResourceConfig.hpp"
#include "InputConfig.hpp"

#if defined(SERVER_BUILD)
#include "ServerResourceManager.hpp"
struct system_context {
    float dt;
    ResourceManager<TextureAsset>& texture_manager;
    ResourceManager<SoundAsset>& sound_manager;
    InputManager& input;
};
#elif defined(CLIENT_BUILD)
#include "ClientResourceManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "InputConfig.hpp"
struct system_context {
    float dt;
    ResourceManager<TextureAsset>& texture_manager;
    ResourceManager<SoundAsset>& sound_manager;
    sf::RenderWindow& window;
    InputManager& input;
};
#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif

struct SerializationContext {
    ResourceManager<TextureAsset>& textureManager;
};
