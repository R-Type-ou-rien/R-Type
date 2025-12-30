#pragma once

#include <functional>
#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "InputManager.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "BackgroundSystem.hpp"
#include "SpawnSystem.hpp"
#include "WindowManager.hpp"
#include "ressource_manager.hpp"

#define SUCCESS 0
#define FAILURE -1
#define WINDOW_H 1000
#define WINDOW_W 1000
#define USER_FUNCTION_SIGNATURE void(ECS & ecs, InputManager & inputs)

class ClientGameEngine {
   private:
    ECS _ecs;
    WindowManager _window_manager;
    InputManager input_manager;
    std::function<USER_FUNCTION_SIGNATURE> _function;
    std::function<USER_FUNCTION_SIGNATURE> _init_function;
    // client network class

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {}
    void setUserFunction(std::function<USER_FUNCTION_SIGNATURE> user_function);
    void setInitFunction(std::function<USER_FUNCTION_SIGNATURE> user_function);

   private:
    void handleEvent();
};
