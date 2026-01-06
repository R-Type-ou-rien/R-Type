#pragma once

#include <functional>
#include <string>

#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "GameEngineBase.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "WindowManager.hpp"

#define SUCCESS 0
#define FAILURE -1
#define WINDOW_H 1000
#define WINDOW_W 1000

class ClientGameEngine : public GameEngineBase<ClientGameEngine> {
   private:
    WindowManager _window_manager;

    public:
        static constexpr bool IsServer = false;

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {}

   private:
    void handleEvent();
    void processNetworkEvents();
};
