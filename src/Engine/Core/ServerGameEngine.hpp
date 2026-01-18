#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

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
#include "LobbyManager.hpp"

#define SUCCESS 0
#define FAILURE -1
class Player;

class ServerGameEngine : public GameEngineBase<ServerGameEngine> {
   private:
    std::shared_ptr<Environment> _env;

    engine::core::LobbyManager _lobbyManager;
    std::map<uint32_t, Entity> _clientToEntityMap;
    std::map<uint32_t, std::shared_ptr<Player>> _players;
    std::set<uint32_t> _pendingFullState;  // Clients waiting for UDP confirmation to receive full state
    void processNetworkEvents();
    void updateActions(ActionPacket& packet, uint32_t clientId);

   public:
    int init();
    int run();
    explicit ServerGameEngine(std::string ip = "");
    ~ServerGameEngine() = default;

    static constexpr bool IsServer = true;
    std::optional<Entity> getLocalPlayerEntity() const { return std::nullopt; }
    engine::core::LobbyManager& getLobbyManager() { return _lobbyManager; }
    std::map<uint32_t, Entity>& getClientToEntityMap() { return _clientToEntityMap; }
};
