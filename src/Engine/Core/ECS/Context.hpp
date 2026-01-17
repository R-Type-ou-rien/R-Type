#pragma once

#include <cstdint>
#include <vector>
#include <unordered_set>
#include "ResourceConfig.hpp"
#include "InputConfig.hpp"

#if defined(SERVER_BUILD)
#include "ServerResourceManager.hpp"
namespace engine::core {
class LobbyManager;
class NetworkEngine;
}  // namespace engine::core
class SceneManager;

struct system_context {
    float dt;
    uint32_t tick;
    ResourceManager<TextureAsset>& texture_manager;
    ResourceManager<SoundAsset>& sound_manager;
    ResourceManager<MusicAsset>& music_manager;
    InputManager& input;
    engine::core::NetworkEngine& network;
    std::vector<uint32_t> active_clients;
    engine::core::LobbyManager* lobby_manager;
    std::unordered_set<uint32_t>* networked_component_types = nullptr;
    class SceneManager* scene_manager = nullptr;
};

#elif defined(CLIENT_BUILD)
#include "ClientResourceManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
struct system_context {
    float dt;
    uint32_t tick;
    ResourceManager<TextureAsset>& texture_manager;
    ResourceManager<SoundAsset>& sound_manager;
    ResourceManager<MusicAsset>& music_manager;
    sf::RenderWindow& window;
    InputManager& input;
    uint32_t player_id;
    class SceneManager* scene_manager = nullptr;
};
#else
#error "You must compile with -DSERVER_BUILD or -DCLIENT_BUILD"
#endif

struct SerializationContext {
    ResourceManager<TextureAsset>& textureManager;
};
