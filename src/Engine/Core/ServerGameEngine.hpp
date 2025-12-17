#include <cstdint>
#include <vector>
#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "InputManager.hpp"
#include "InputSystem.hpp"
#include "Network/Server/Server.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "BackgroundSystem.hpp"
#include "WindowManager.hpp"
#include "ressource_manager.hpp"
#include "Components/StandardComponents.hpp"
#include "Network/Client/Client.hpp"
#include "NetworkSystem/ComponentSenderSystem.hpp"
#include "PatternSystem/PatternSystem.hpp"
#include "CollisionSystem.hpp"
#include "ActionScriptSystem.hpp"

#pragma once

class ServerGameEngine {
   private:
    ECS _ecs;
    Server _network_server;
    bool _has_game_start = false;
    InputManager _input_manager;
    std::vector<uint32_t> _players;
    std::function<void(ECS& ecs)> _function;
    std::function<void(ECS& ecs)> _init_function;

   public:
    int init();
    int run();
    explicit ServerGameEngine();
    ~ServerGameEngine();
    void setUserFunction(std::function<void(ECS& ecs)> user_function);
    void setInitFunction(std::function<void(ECS& ecs)> user_function);

   private:
    void handleNetworkMessages();
    void execCorrespondingFunction(GameEvents event, coming_message c_msg);
    std::function<void(std::uint32_t)> _onPlayerConnect;

   public:
    void setOnPlayerConnect(std::function<void(std::uint32_t)> callback);
    ECS& getECS();
};