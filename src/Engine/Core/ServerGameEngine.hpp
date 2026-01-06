#pragma once

#include <functional>
#include <string>

#include "Components/NetworkComponents.hpp"
#include "ECS/ECS.hpp"
#include "GameEngineBase.hpp"
#include "SpawnSystem.hpp"
#include "InputConfig.hpp"
#include "PhysicsSystem.hpp"
#include "ResourceConfig.hpp"
#include "CollisionSystem.hpp"
#include "ActionScriptSystem.hpp"
#include "PatternSystem/PatternSystem.hpp"
#include "ComponentSenderSystem/ComponentSenderSystem.hpp"
#include "ServerResourceManager.hpp"

#define SUCCESS 0
#define FAILURE -1

class ServerGameEngine : public GameEngineBase<ServerGameEngine> {
    public:
        static constexpr bool IsServer = true;

   public:
    int init();
    int run();
    explicit ServerGameEngine();
    ~ServerGameEngine() = default;

    private:
        void processNetworkEvents();
        void updateActions(ActionPacket& packet);
};