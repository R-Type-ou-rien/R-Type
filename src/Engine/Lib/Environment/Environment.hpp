#include "ECS.hpp"
#include "ResourceConfig.hpp"
#include "slot_map/slot_map.hpp"

enum class SpawnPolicy { AUTHORITATIVE, PREDICTED, LOCAL_ONLY };

enum class EnvMode { SERVER, CLIENT, STANDALONE };

class Environment {
   private:
    ECS& _ecs;
    ResourceManager<TextureAsset>& _textures;
    EnvMode _mode;

   public:
    Environment(ECS& ecs, ResourceManager<TextureAsset>& textures, EnvMode mode)
        : _ecs(ecs), _textures(textures), _mode(mode) {}

    handle_t<TextureAsset> loadTexture(const std::string& path) {
        if (!_textures.is_loaded(path)) {
            return _textures.load(path, TextureAsset(path));
        }
        return _textures.get_handle(path).value();
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