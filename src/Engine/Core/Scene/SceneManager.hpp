#pragma once

#include "LevelConfig.hpp"
#include "../ECS/Registry/registry.hpp"
#include "../../Lib/Components/LobbyIdComponent.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <any>

class SceneManager {
   public:
    using CreatorFn = std::function<void(Registry&, Entity, const std::unordered_map<std::string, std::any>&)>;

    explicit SceneManager(Registry& registry) : _registry(registry) {
        _prefabs["Background"] = [](Registry& reg, Entity e, const std::unordered_map<std::string, std::any>& props) {
            // A faire
            if (props.count("texture")) {
                // a faire}
            }
        };
        _prefabs["Music"] = [](Registry& reg, Entity e, const std::unordered_map<std::string, std::any>& props) {
            // A faire
            if (props.count("track")) {
                // a faire}
            }
        };
    }

    void registerPrefab(const std::string& name, CreatorFn creator) { _prefabs[name] = creator; }

    void setCurrentLobbyId(uint32_t lobbyId) { _currentLobbyId = lobbyId; }

    void loadScene(const LevelConfig& config) {
        std::cout << "[SceneManager] Loading scene with " << config.entities.size() << " entities for lobby "
                  << _currentLobbyId << std::endl;

        // 1. Create Background Entity
        if (!config.background_texture.empty()) {
            _createEntity("Background", {{"texture", config.background_texture}});
        }

        // rajouter le son
        if (!config.music_track.empty()) {
            _createEntity("Music", {{"track", config.music_track}});
        }

        // 2. Spawn Entities
        for (const auto& entConfig : config.entities) {
            std::cout << "[SceneManager] Creating entity of type: " << entConfig.type << std::endl;
            _createEntity(entConfig.type, entConfig.properties);
        }
    }

   private:
    void _createEntity(const std::string& type, const std::unordered_map<std::string, std::any>& props) {
        if (_prefabs.find(type) == _prefabs.end()) {
            std::cerr << "Warning: Unknown entity type '" << type << "'" << std::endl;
            return;
        }

        Entity e = _registry.createEntity();
        _prefabs[type](_registry, e, props);

        // Tag entity with lobby ID
        if (_currentLobbyId != 0) {
            _registry.addComponent<LobbyIdComponent>(e, {_currentLobbyId});
        }
    }

    Registry& _registry;
    std::unordered_map<std::string, CreatorFn> _prefabs;
    uint32_t _currentLobbyId = 0;
};
