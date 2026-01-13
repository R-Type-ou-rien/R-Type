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
#include "BackgroundSystem.hpp"
#include "ResourceConfig.hpp"
#include "WindowManager.hpp"
#include "PredictionSystem.hpp"

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
    std::unique_ptr<PredictionSystem> _predictionSystem;
    PhysicsSimulationCallback _physicsLogic;

   public:
    static constexpr bool IsServer = false;

   public:
    int init();
    int run();
    explicit ClientGameEngine(std::string window_name = "Default Name");
    ~ClientGameEngine() {}
    void setPredictionLogic(PhysicsSimulationCallback logic) { _physicsLogic = logic; }

    std::optional<Entity> getLocalPlayerEntity() const {
        if (!_localPlayerEntity.has_value())
            return std::nullopt;

        auto it = _networkToLocalEntity.find(_localPlayerEntity.value());
        if (it != _networkToLocalEntity.end()) {
            return it->second;
        }
        return std::nullopt;
    }

   private:
    void handleEvent();
    void processNetworkEvents();
    void applyLocalInputs(Entity playerEntity);
    void reconcile(Entity playerEntity, const transform_component_s& serverState, uint32_t serverTick);
};
