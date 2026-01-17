#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include <optional>
#include "ECS/EcsType.hpp"
#include "ECS/ECS.hpp"
#include "ECS/ISystem.hpp"
#include "GameEngineBase.hpp"
#include "InputConfig.hpp"
#include "InputSystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "NewRenderSystem/NewRenderSystem.hpp"
#include "AnimationSystem/AnimationSystem.hpp"
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "WindowManager.hpp"

#define SUCCESS 0
#define FAILURE -1
#define WINDOW_H 1080
#define WINDOW_W 1920

class ClientGameEngine : public GameEngineBase<ClientGameEngine> {
   private:
    WindowManager _window_manager;
    uint32_t _serverId = 0;
    uint32_t _clientId = 0;
    std::optional<Entity> _localPlayerEntity;

   public:
    static constexpr bool IsServer = false;

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {}

    std::optional<Entity> getLocalPlayerEntity() const {
        if (!_localPlayerEntity.has_value())
            return std::nullopt;

        // Convert network GUID to local entity ID
        auto it = _networkToLocalEntity.find(_localPlayerEntity.value());
        if (it != _networkToLocalEntity.end()) {
            return it->second;
        }
        return std::nullopt;
    }

   private:
    void handleEvent();
    void processNetworkEvents();
};
