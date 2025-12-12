#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <functional>

#include "ECS.hpp"
#include "ISystem.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "WindowManager.hpp"
#include "ressource_manager.hpp"

#define SUCCESS 0
#define FAILURE -1
#define WINDOW_H 1000
#define WINDOW_W 1000

class ClientGameEngine {
   private:
    ECS _ecs;
    WindowManager _window_manager;
    std::function<void(ECS& ecs)> _function;
    std::function<void(ECS& ecs)> _init_function;
    // client network class

   public:
    int init();
    int run();
    ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine(){};
    void setUserFunction(std::function<void(ECS& ecs)> user_function);
    void setInitFunction(std::function<void(ECS& ecs)> user_function);

   private:
    void handleEvent();
};