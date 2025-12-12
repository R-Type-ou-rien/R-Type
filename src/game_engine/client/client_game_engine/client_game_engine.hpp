#pragma once

#include <functional>
#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "ecs/client/InputSystem/InputSystem.hpp"
#include "ecs/client/RenderSystem/RenderSystem.hpp"
#include "ecs/common/ECS.hpp"
#include "ecs/common/ISystem.hpp"
#include "ecs/common/MoveSystem/MoveSystem.hpp"
#include "ecs/common/ressource_manager/ressource_manager.hpp"
#include "game_engine/client/window_manager/window_manager.hpp"

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

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {}
    void setUserFunction(std::function<void(ECS& ecs)> user_function);
    void setInitFunction(std::function<void(ECS& ecs)> user_function);

   private:
    void handleEvent();
};
