#include "Hash/Hash.hpp"
#include "ClientGameEngine.hpp"
#include <SFML/Graphics/RenderTexture.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include "CollisionSystem.hpp"
#include "ActionScriptSystem.hpp"
#include "Components/NetworkComponents.hpp"
#include "Components/StandardComponents.hpp"
#include "Network/Network.hpp"
#include "Network/Network.hpp"
#include "NetworkSystem/ComponentSenderSystem.hpp"
#include "PatternSystem/PatternSystem.hpp"
#include "PatternSystem/PatternSystem.hpp"
#include "registry.hpp"
#include "Systems/PhysicsSystem.hpp"

ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name) {}

int ClientGameEngine::init() {
    InputManager input_manager;

    _network_client.Connect("127.0.0.1", 4040);

    _ecs._textureManager.load_resource("content/sprites/r-typesheet42.gif",
                                       sf::Texture("content/sprites/r-typesheet42.gif"));
    _ecs._textureManager.load_resource("content/sprites/r-typesheet1.gif",
                                       sf::Texture("content/sprites/r-typesheet1.gif"));
    _ecs._textureManager.load_resource("content/sprites/background-R-Type.png",
                                       sf::Texture("content/sprites/background-R-Type.png"));

    registerNetworkComponent<BoxCollisionComponent>();
    registerNetworkComponent<BackgroundComponent>();
    registerNetworkComponent<sprite2D_component_s>();
    registerNetworkComponent<transform_component_s>();
    registerNetworkComponent<PatternComponent>();
    registerNetworkComponent<Velocity2D>();
    registerNetworkComponent<TagComponent>();
    registerNetworkComponent<ActionScript>();
    registerNetworkComponent<Shooter>();
    registerNetworkComponent<Projectile>();
    registerNetworkComponent<Scroll>();
    registerNetworkComponent<NetworkIdentity>();
    registerNetworkComponent<ComponentPacket>();

    // Manual registration for BackgroundComponent to reload texture
    uint32_t bgTypeId = Hash::fnv1a(BackgroundComponent::name);
    _deserializers[bgTypeId] = [this](Registry& reg, Entity e, const std::vector<uint8_t>& data) {
        BackgroundComponent component{};
        component.texture_handle = handle_t<sf::Texture>::Null();
        Serializer<BackgroundComponent>::deserialize(component, data);

        // Reload texture handle (hardcoded for now as ID is missing in packet)
        // If we had a texture ID in BackgroundComponent, we would use it here.
        // Falling back to default R-Type background.
        std::string bgPath = "content/sprites/background-R-Type.png";
        if (_ecs._textureManager.is_loaded(bgPath)) {
            component.texture_handle = _ecs._textureManager.get_handle(bgPath).value();
        } else {
            // Should verify if loaded, but init() usually loads it.
            // If not loaded, we might have an invalid handle, but safer than random server memory.
        }

        if (reg.hasComponent<BackgroundComponent>(e)) {
            // Preserve x_offset to allow smooth scrolling
            auto& existing = reg.getComponent<BackgroundComponent>(e);
            existing.scroll_speed = component.scroll_speed;

            // Update texture if newly loaded (though it's hardcoded)
            // But CRITICALLY, do not overwrite x_offset with component.x_offset (which is 0)
            if (component.texture_handle != handle_t<sf::Texture>::Null()) {
                existing.texture_handle = component.texture_handle;
            }
        } else {
            reg.addComponent<BackgroundComponent>(e, component);
        }
    };

    _ecs.systems.addSystem<BackgroundSystem>();
    _ecs.systems.addSystem<RenderSystem>();
    _ecs.systems.addSystem<InputSystem>(_ecs.input);
    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<ComponentSenderSystem>();

    if (_init_function)
        _init_function(_ecs);

    // if mode local
    _ecs.systems.addSystem<PhysicsSystem>();
    return 0;
}

void ClientGameEngine::setUserFunction(std::function<void(ECS& ecs)> user_function) {
    _function = user_function;
    return;
}

void ClientGameEngine::setInitFunction(std::function<void(ECS& ecs)> user_function) {
    _init_function = user_function;
    return;
}

void ClientGameEngine::handleEvent() {
    while (std::optional<sf::Event> event = _window_manager.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window_manager.getWindow().close();
        if (event->is<sf::Event::FocusLost>())
            _ecs.input.setWindowHasFocus(false);
        if (event->is<sf::Event::FocusGained>())
            _ecs.input.setWindowHasFocus(true);
    }
}

int ClientGameEngine::run() {
    sf::Clock clock;
    system_context context = {0,
                              _ecs._textureManager,
                              _window_manager.getWindow(),
                              _ecs.input,
                              _network_client,
                              std::nullopt,
                              std::nullopt,
                              _identity.id};

    this->init();
    while (_window_manager.isOpen()) {
        handleNetworkMessages();
        context.dt = clock.restart().asSeconds();
        handleEvent();
        _window_manager.clear();
        if (_has_game_started) {
            if (_function) {
                _function(_ecs);
            }
            _ecs.update(context);
        }
        _window_manager.display();
    }
    return 0;
}

void ClientGameEngine::handleNetworkMessages() {
    if (_network_client.IsConnected()) {
        int processed = 0;
        while (processed < 100) {
            coming_message c_msg = _network_client.ReadIncomingMessage();
            if (c_msg.id == GameEvents::NONE) {
                break;
            }
            execCorrespondingFunction(c_msg.id, c_msg);
            processed++;
        }
    } else {
        throw std::logic_error("Client couldn't connect to the server");
    }
}

void ClientGameEngine::getID(coming_message msg) {
    msg.msg >> _identity.id;
    _network_client.SetID(_identity.id);
    std::cout << "RECEIVED ID " << _identity.id << std::endl;
}

void ClientGameEngine::getRoom(coming_message msg) {
    msg.msg >> _identity.lobby;
}

void ClientGameEngine::updateEntity(coming_message msg) {
    ComponentPacket packet;
    Entity current_entity;

    msg.msg >> packet;
    std::cout << "packet guid: " << packet.entity_guid << " component type: " << packet.component_type << std::endl;
    if (_networkToLocalEntity.find(packet.entity_guid) != _networkToLocalEntity.end()) {
        std::cout << "get local entity from network" << std::endl;
        current_entity = _networkToLocalEntity[packet.entity_guid];
    } else {
        std::cout << "create local entity from network" << std::endl;
        current_entity = _ecs.registry.createEntity();
        _networkToLocalEntity[packet.entity_guid] = current_entity;
        _ecs.registry.addComponent<NetworkIdentity>(current_entity, {packet.entity_guid, msg.clientID});
    }

    // Rubber Banding Fix: Ignore server updates for our own entity's movement (Transform/Velocity)
    // We trust our local prediction for display smoothness.
    if (_ecs.registry.hasComponent<NetworkIdentity>(current_entity)) {
        auto& netId = _ecs.registry.getComponent<NetworkIdentity>(current_entity);
        if (netId.owner_user_id == _identity.id) {
            // Recalculate hashes locally since we can't easily access the component type from just the packet type ID
            // without a lookup But we know StandardComponents defines them.
            if (packet.component_type == Hash::fnv1a("Transform") || packet.component_type == Hash::fnv1a("Velocity")) {
                return;
            }
        }
    }

    if (_deserializers.find(packet.component_type) != _deserializers.end()) {
        std::cout << "export data from network" << std::endl;
        _deserializers[packet.component_type](_ecs.registry, current_entity, packet.data);
    } else {
        std::cerr << "Unknown component type: " << packet.component_type << " for entity GUID " << packet.entity_guid
                  << std::endl;
    }
}

void ClientGameEngine::execCorrespondingFunction(GameEvents event, coming_message c_msg) {
    switch (event) {
        case (GameEvents::S_SNAPSHOT):
            updateEntity(c_msg);
            break;
        case (GameEvents::S_SEND_ID):
            std::cout << "EVENT SEND ID" << std::endl;
            getID(c_msg);
            // Redundant send to ensure server gets UDP endpoint despite packet loss
            for (int i = 0; i < 20; ++i) {
                _network_client.AddMessageToServer(GameEvents::C_CONFIRM_UDP, _identity.id, NULL);
            }
            break;

        case (GameEvents::S_CONFIRM_UDP):
            std::cout << "UDP CONFIMED BY THE SERVER" << std::endl;
            break;

        case (GameEvents::S_REGISTER_OK):
            std::cout << "REGISTER OK TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_REGISTER_KO):
            std::cout << "REGISTER KO TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_INVALID_TOKEN):
            std::cout << "INVALID TOKEN TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_LOGIN_OK):
            std::cout << "LOGIN OK TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_LOGIN_KO):
            std::cout << "LOGIN KO TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_ROOMS_LIST):
            std::cout << "ROOMS LIST TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_ROOM_JOINED):
            std::cout << "ROOM JOINED" << std::endl;
            getRoom(c_msg);
            break;

        case (GameEvents::S_PLAYER_JOINED):
            std::cout << "PLAYER JOINED !!!!" << std::endl;
            break;

        case (GameEvents::S_ROOM_NOT_JOINED):
            std::cout << "ROOM NOT JOINED TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_PLAYER_LEAVE):
            std::cout << "PLAYER LEAVE TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_PLAYER_KICKED):
            std::cout << "PLAYER KICKED TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_NEW_HOST):
            std::cout << "NEW HOST TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_CONFIRM_NEW_LOBBY):
            std::cout << "CONFIRM NEW LOBBY TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_READY_RETURN):
            std::cout << "READY RETURN TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_GAME_START):
            _has_game_started = true;
            std::cout << "GAME START" << std::endl;
            break;

        case (GameEvents::S_CANCEL_READY_BROADCAST):
            std::cout << "CANCEL READY BROADCAST TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_TEAM_CHAT):
            std::cout << "TEAM CHAT TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_VOICE_RELAY):
            std::cout << "VOICE RELAY TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_PLAYER_DEATH):
            std::cout << "PLAYER DEATH TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_SCORE_UPDATE):
            std::cout << "SCORE UPDATE TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_GAME_OVER):
            std::cout << "GAME OVER TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_RETURN_TO_LOBBY):
            std::cout << "RETURN TO LOBBY TO IMPLEMENT" << std::endl;
            break;

        default:
            // std::cout << "EVENT " << uint32_t(event) << " IS NOT IMPLEMENTED" << std::endl;
            break;
    }
}
