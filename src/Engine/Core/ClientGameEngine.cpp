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
#include "registry.hpp"


ClientGameEngine::ClientGameEngine(std::string window_name) : _window_manager(WINDOW_W, WINDOW_H, window_name) {}

int ClientGameEngine::init() {
    InputManager input_manager;

    _network_client.Connect("127.0.0.1", 4242);    

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
    system_context context = {
        0,
        _ecs._textureManager,
        _window_manager.getWindow(),
        _ecs.input,
        _network_client,
        std::nullopt,
        std::nullopt,
        _identity.id
    };

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

void ClientGameEngine::handleNetworkMessages()
{
    if (_network_client.IsConnected()) {
        // send authentification
        std::cout << "user connected" << std::endl;
        coming_message c_msg = _network_client.ReadIncomingMessage();
            // send id -> message udp -> S_CONFIRM_UDP
            // S_ROOM_JOINED
            // player qui join -> S_PLAYER_JOINED -> 
            // S_GAME_START
        if (c_msg.id != GameEvents::NONE) {
           execCorrespondingFunction(c_msg.id, c_msg);
        }
    } else {
        throw std::logic_error("Client couldn't connect to the server");
    }
}

void ClientGameEngine::getID(coming_message msg)
{
    msg.msg >> _identity.id;
}

void ClientGameEngine::getRoom(coming_message msg)
{
    msg.msg >> _identity.lobby;
}

void ClientGameEngine::updateEntity(coming_message msg)
{
    ComponentPacket packet;
    Entity current_entity;

    msg.msg >> packet;

    if (_networkToLocalEntity.find(packet.entity_guid) != _networkToLocalEntity.end()) {
        current_entity = _networkToLocalEntity[packet.entity_guid];
    } else {
        current_entity = _ecs.registry.createEntity();
        _networkToLocalEntity[packet.entity_guid] = current_entity;
        _ecs.registry.addComponent<NetworkIdentity>(current_entity, {packet.entity_guid, msg.clientID});
    }

    if (_deserializers.find(packet.component_type) != _deserializers.end()) {
        _deserializers[packet.component_type](_ecs.registry, current_entity, packet.data);
    }
    
}

void ClientGameEngine::execCorrespondingFunction(GameEvents event, coming_message c_msg)
{
    switch (event) {
        case (GameEvents::S_SNAPSHOT):
            updateEntity(c_msg);
            break;
        case (GameEvents::S_SEND_ID):
            getID(c_msg);
            _network_client.AddMessageToServer(GameEvents::C_CONFIRM_UDP, _identity.id, NULL);
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
            getRoom(c_msg);
            std::cout << "ROOM JOINED" << std::endl;
            break;

        case (GameEvents::S_PLAYER_JOINED):
            std::cout << "A PLAYER JOINED !!!!" << std::endl;
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
            std::cout << "EVENT " << uint32_t(event) << " IS NOT IMPLEMENTED" << std::endl;
            break; 
    }
}
