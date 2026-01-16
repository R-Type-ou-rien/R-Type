#include "ECS.hpp"
#include "ResourceConfig.hpp"
#include "slot_map/slot_map.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <utility>

enum class SpawnPolicy { AUTHORITATIVE, PREDICTED, LOCAL_ONLY };

enum class EnvMode { SERVER, CLIENT, STANDALONE };

class Environment {
   private:
    ECS& _ecs;
    ResourceManager<TextureAsset>& _textures;
    ResourceManager<SoundAsset>& _sounds;
    ResourceManager<MusicAsset>& _musics;
    EnvMode _mode;

   public:
    Environment(ECS& ecs, ResourceManager<TextureAsset>& textures, ResourceManager<SoundAsset>& sounds,
                ResourceManager<MusicAsset>& musics, EnvMode mode)
        : _ecs(ecs), _textures(textures), _sounds(sounds), _musics(musics), _mode(mode) {}

    handle_t<TextureAsset> loadTexture(const std::string& path) {
        if (!_textures.is_loaded(path)) {
            return _textures.load(path, TextureAsset(path));
        }
        return _textures.get_handle(path).value();
    }

    void loadSound(const std::string& name, const std::string& path) {
#if defined(CLIENT_BUILD)
        sf::SoundBuffer buffer;
        if (buffer.loadFromFile(path)) {
            _sounds.load(name, buffer);
            std::cout << "[AUDIO] Loaded sound: " << name << " from " << path << std::endl;
        } else {
            std::cerr << "[AUDIO] FAILED to load sound: " << path << std::endl;
        }
#else
        _sounds.load(name, SoundAsset(path));
#endif
    }

    void loadMusic(const std::string& name, const std::string& path) {
#if defined(CLIENT_BUILD)
        _musics.load(name, path);
        std::cout << "[AUDIO] Registering music path: " << name << " from " << path << std::endl;
#else
        _musics.load(name, MusicAsset(path));
#endif
    }

    void loadGameResources(const std::string& jsonPath) {
        std::ifstream f(jsonPath);
        if (!f.is_open()) {
            std::cerr << "[ERROR] Could not open resource file: " << jsonPath << std::endl;
            return;
        }

        try {
            nlohmann::json data = nlohmann::json::parse(f);

            // Load textures
            if (data.contains("textures") && data["textures"].is_array()) {
                for (const auto& texture : data["textures"]) {
                    if (texture.contains("path")) {
                        loadTexture(texture["path"]);
                        std::cout << "[RESOURCE] Loaded texture: " << texture["path"] << std::endl;
                    }
                }
            }

            if (data.contains("sounds") && data["sounds"].is_array()) {
                for (const auto& sound : data["sounds"]) {
                    if (sound.contains("name") && sound.contains("path")) {
                        loadSound(sound["name"], sound["path"]);
                        std::cout << "[RESOURCE] Loaded sound: " << sound["name"] << std::endl;
                    }
                }
            }

            if (data.contains("music") && data["music"].is_array()) {
                for (const auto& music : data["music"]) {
                    if (music.contains("name") && music.contains("path")) {
                        loadMusic(music["name"], music["path"]);
                        std::cout << "[RESOURCE] Loaded music: " << music["name"] << std::endl;
                    }
                }
            }
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "[ERROR] JSON parse error in " << jsonPath << ": " << e.what() << std::endl;
        }
    }

    template <typename ActorType, typename... Args>
    std::unique_ptr<ActorType> spawn(SpawnPolicy policy, Args&&... args) {
        bool shouldSpawn = false;

        if (_mode == EnvMode::STANDALONE) {
            shouldSpawn = true;
        } else {
            switch (policy) {
                case SpawnPolicy::AUTHORITATIVE:
                    shouldSpawn = (_mode == EnvMode::SERVER);
                    break;
                case SpawnPolicy::PREDICTED:
                    shouldSpawn = true;
                    break;
                case SpawnPolicy::LOCAL_ONLY:
                    shouldSpawn = (_mode == EnvMode::CLIENT);
                    break;
            }
        }

        if (shouldSpawn) {
            return std::make_unique<ActorType>(_ecs, _textures, std::forward<Args>(args)...);
        }
        return nullptr;
    }

    bool isServer() const { return (_mode == EnvMode::SERVER); }

    bool isClient() const { return (_mode == EnvMode::CLIENT); }

    ECS& getECS() { return _ecs; }
};
