#include "ServerGameEngine.hpp"
#include <optional>


int ServerGameEngine::init()
{
    _network_server.WaitForClientConnection();

    _ecs.systems.addSystem<BoxCollision>();
    _ecs.systems.addSystem<ActionScriptSystem>();
    _ecs.systems.addSystem<PatternSystem>();
    _ecs.systems.addSystem<ComponentSenderSystem>();
    _ecs.systems.addSystem<PhysicsSystem>();

    if (_init_function)
        _init_function(_ecs);

    return 0;
}

void ServerGameEngine::setUserFunction(std::function<void(ECS& ecs)> user_function) {
    _function = user_function;
    return;
}

void ServerGameEngine::setInitFunction(std::function<void(ECS& ecs)> user_function) {
    _init_function = user_function;
    return;
}

int ServerGameEngine::run() {
    sf::Clock clock;
    system_context context = {
        0,
        _ecs._textureManager,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        _network_server
    };

    this->init();
    while (1) {
        context.dt = clock.restart().asSeconds();
        if (_function)
            _function(_ecs);
        _ecs.update(context);
    }
    return 0;
}

void ServerGameEngine::handleNetworkMessages()
{
    
}

void ServerGameEngine::execCorrespondingFunction(GameEvents event, coming_message c_msg)
{
    switch (event) {
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
            std::cout << "ROOM JOINED TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_PLAYER_JOINED):
            std::cout << "PLAYER JOINED TO IMPLEMENT" << std::endl;
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
            std::cout << "GAME START TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_CANCEL_READY_BROADCAST):
            std::cout << "CANCEL READY BROADCAST TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_QUIT_LOBBY_BROADCAST):
            std::cout << "QUIT LOBBY BROADCAST TO IMPLEMENT" << std::endl;
            break;

        case (GameEvents::S_SNAPSHOT):
            std::cout << "SNAPSHOT TO IMPLEMENT" << std::endl;
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
