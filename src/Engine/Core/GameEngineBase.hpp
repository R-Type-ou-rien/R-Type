#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_set>

#include "ECS/ECS.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "Context.hpp"
#include "Components/serialize/NetworkTraits.hpp"

#include "Environment/Environment.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/LevelConfig.hpp"

#define SUCCESS 0
#define FAILURE -1
#define USER_FUNCTION_SIGNATURE void(std::shared_ptr<Environment> env, InputManager & inputs)
#define DESERIALIZER_FUNCTION std::function<void(Registry&, Entity, const std::vector<uint8_t>&)>

template <class Derived>
class GameEngineBase {
   protected:
    ECS _ecs;
    SceneManager _scene_manager;
    InputManager input_manager;
    ResourceManager<TextureAsset> _texture_manager;
    ResourceManager<SoundAsset> _sound_manager;
    ResourceManager<MusicAsset> _music_manager;
    std::function<USER_FUNCTION_SIGNATURE> _loop_function;
    std::function<USER_FUNCTION_SIGNATURE> _init_function;

    std::shared_ptr<engine::core::NetworkEngine> _network;
    std::unordered_map<uint32_t, DESERIALIZER_FUNCTION> _deserializers;
    std::unordered_set<uint32_t> _networkedComponentTypes;  // Hash of component types that can be sent over network
    std::unordered_map<uint32_t, Entity> _networkToLocalEntity;
    uint32_t _currentTick = 0;

   public:
    explicit GameEngineBase() : _network(nullptr), _scene_manager(_ecs.registry) {}
    ~GameEngineBase() = default;

    SceneManager& getSceneManager() { return _scene_manager; }
    ResourceManager<SoundAsset>& getSoundManager() { return _sound_manager; }
    ResourceManager<MusicAsset>& getMusicManager() { return _music_manager; }
    std::unordered_set<uint32_t>& getNetworkedComponentTypes() { return _networkedComponentTypes; }

    template <typename T>
    void registerNetworkComponent() {
        uint32_t typeId = Hash::fnv1a(T::name);

        _networkedComponentTypes.insert(typeId);  // Track this type as networked

        _deserializers[typeId] = [this](Registry& reg, Entity e, const std::vector<uint8_t>& data) {
            size_t offset = 0;
            try {
                T component = serialize::ComponentTraits<T>::deserialize(data, offset, _texture_manager);

                if (reg.hasComponent<T>(e)) {
                    reg.getComponent<T>(e) = component;
                } else {
                    reg.addComponent<T>(e, component);
                }
            } catch (const std::exception& e_log) {
                std::cerr << "[Network] Error deserializing component " << T::name << ": " << e_log.what() << std::endl;
            }
        };
    }

    void processComponentPacket(uint32_t entity_guid, uint32_t component_hash, const std::vector<uint8_t>& data,
                                uint32_t ownerId = 0) {
        Entity current_entity;
        auto it = _networkToLocalEntity.find(entity_guid);

        if (it != _networkToLocalEntity.end()) {
            current_entity = it->second;
        } else {
            current_entity = _ecs.registry.createEntity();
            _networkToLocalEntity[entity_guid] = current_entity;
            _ecs.registry.addComponent<NetworkIdentity>(current_entity, {entity_guid, ownerId});
        }

        auto dest_it = _deserializers.find(component_hash);
        if (dest_it != _deserializers.end()) {
            dest_it->second(_ecs.registry, current_entity, data);
        } else {
            std::cerr << "[Network] Warning: No deserializer registered for component hash: " << component_hash
                      << std::endl;
        }
    }

    int init() { return static_cast<Derived*>(this)->init(); }

    int run() { return static_cast<Derived*>(this)->run(); }

    void setLoopFunction(std::function<USER_FUNCTION_SIGNATURE> user_function) {
        _loop_function = user_function;
        return;
    }

    void setInitFunction(std::function<USER_FUNCTION_SIGNATURE> user_function) {
        _init_function = user_function;
        return;
    }

    void clearNetworkState() { _networkToLocalEntity.clear(); }

    ECS& getECS() { return _ecs; }
    ResourceManager<TextureAsset>& getTextureManager() { return _texture_manager; }
    InputManager& getInputManager() { return input_manager; }
    engine::core::NetworkEngine& getNetwork() { return *_network; }

   protected:
    void processNetworkEvents(Environment& env) { static_cast<Derived*>(this)->processNetworkEvents(env); }
};