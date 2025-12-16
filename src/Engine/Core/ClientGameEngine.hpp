#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "InputSystem.hpp"
#include "Components/StandardComponents.hpp"
#include "Components/ComponentSerializer.hpp"
#include "Network/Network.hpp"
#include "Network/Server/Server.hpp"
#include "RenderSystem.hpp"
#include "BackgroundSystem.hpp"
#include "WindowManager.hpp"
#include "ressource_manager.hpp"
#include "Components/StandardComponents.hpp"
#include "Network/Client/Client.hpp"

#define SUCCESS 0
#define FAILURE -1
#define WINDOW_H 1000
#define WINDOW_W 1000

struct client_identity {
    uint32_t id;
    std::string username = "";
    lobby_in_info lobby;
};

class ClientGameEngine {
   private:
    ECS _ecs;
    WindowManager _window_manager;
    Client _network_client;
    client_identity _identity;
    bool _has_game_started = false;

    using ComponentDeserializer = std::function<void(Registry&, Entity, const std::vector<uint8_t>&)>;
    std::map<uint32_t, ComponentDeserializer> _deserializers;
    std::map<uint32_t, Entity> _networkToLocalEntity;

    std::function<void(ECS& ecs)> _function;
    std::function<void(ECS& ecs)> _init_function;

   public:
    template <typename T>
    void registerNetworkComponent() {
        uint32_t typeId = Hash::fnv1a(T::name);

        _deserializers[typeId] = [](Registry& reg, Entity e, const std::vector<uint8_t>& data) {
            T component;
            // logic relying on memcy size check removed, delegated to serializer

            Serializer<T>::deserialize(component, data);

            if (reg.hasComponent<T>(e)) {
                reg.getComponent<T>(e) = component;
            } else {
                reg.addComponent<T>(e, component);
            }
        };
    }

    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {};
    void setUserFunction(std::function<void(ECS& ecs)> user_function);
    void setInitFunction(std::function<void(ECS& ecs)> user_function);

   private:
    void handleEvent();
    void handleNetworkMessages();
    void getID(coming_message msg);
    void getRoom(coming_message msg);
    void updateEntity(coming_message c_msg);
    void execCorrespondingFunction(GameEvents event, coming_message c_msg);
};
