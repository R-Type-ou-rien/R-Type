#pragma once

#include <functional>
#include <string>
#include "ECS/ECS.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "Components/serialize/NetworkTraits.hpp"
#include "Context.hpp"
#include "Environment/Environment.hpp"
#include "NetworkEngine/NetworkEngine.hpp"

#define SUCCESS 0
#define FAILURE -1
#define USER_FUNCTION_SIGNATURE void(Environment & env, InputManager & inputs)
#define DESERIALIZER_FUNCTION std::function<void(Registry&, Entity, const std::vector<uint8_t>&)>

enum class GameState { MENU, LOBBY, GAME, PAUSE, GAME_OVER };

template <class Derived>
class GameEngineBase {
   protected:
    GameState _gameState = GameState::MENU;
    ECS _ecs;
    InputManager input_manager;
    ResourceManager<TextureAsset> _texture_manager;
    std::function<USER_FUNCTION_SIGNATURE> _loop_function;
    std::function<USER_FUNCTION_SIGNATURE> _init_function;

    std::unique_ptr<engine::core::NetworkEngine> _network;
    std::unordered_map<uint32_t, DESERIALIZER_FUNCTION> _deserializers;
    std::unordered_map<uint32_t, Entity> _networkToLocalEntity;
    uint32_t _currentTick = 0;

   public:
    explicit GameEngineBase() : _network(nullptr) {}
    ~GameEngineBase() = default;

    void setGameState(GameState state) { _gameState = state; }
    GameState getGameState() const { return _gameState; }

    template <typename T>
    void registerNetworkComponent() {
        uint32_t typeId = Hash::fnv1a(T::name);

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
            if (!Derived::IsServer) {
                std::cout << "[Network] Created new local entity " << current_entity << " for GUID " << entity_guid
                          << std::endl;
            }
        }

        auto dest_it = _deserializers.find(component_hash);
        if (dest_it != _deserializers.end())
            dest_it->second(_ecs.registry, current_entity, data);
        else
            std::cerr << "[Network] Warning: No deserializer registered for component hash: " << component_hash
                      << std::endl;
    }

    void clearNetworkState() { _networkToLocalEntity.clear(); }

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
};