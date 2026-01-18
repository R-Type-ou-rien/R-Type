#include "DestructionSystem.hpp"
#include <iostream>
#include <variant>
#include <vector>
#include <memory>
#include "Components/StandardComponents.hpp"
#include "Components/NetworkComponents.hpp"
#include "Context.hpp"
#include "Network.hpp"
#include "../../../RType/Common/Components/spawn.hpp"
#include "../../../RType/Common/Components/game_timer.hpp"
#include "../../../RType/Common/Components/scripted_spawn.hpp"
#include "../../../RType/Common/Systems/health.hpp"

#if defined(SERVER_BUILD)
#include "NetworkEngine/NetworkEngine.hpp"
#include "../../../Network/Server/Server.hpp"
#include "ServerGameEngine.hpp"
#endif

void DestructionSystem::update(Registry& registry, system_context context) {
    auto entities = registry.getEntities<PendingDestruction>();

    // Copy entities to separate vector to avoid iterator invalidation during destruction
    std::vector<Entity> to_destroy;
    for (auto entity : entities) {
        // Protect system entities from accidental destruction
        if (registry.hasComponent<EnemySpawnComponent>(entity) || registry.hasComponent<GameTimerComponent>(entity) ||
            registry.hasComponent<ScriptedSpawnComponent>(entity)) {
            // Remove PendingDestruction to keep system entities alive
            registry.removeComponent<PendingDestruction>(entity);
            continue;
        }
        to_destroy.push_back(entity);
    }

    if (to_destroy.empty())
        return;

#if defined(SERVER_BUILD)
    auto network_instance = context.network.getNetworkInstance();
    std::shared_ptr<network::Server> server = nullptr;

    if (std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
        server = std::get<std::shared_ptr<network::Server>>(network_instance);
    }

    if (server && context.lobby_manager) {
        auto& lobbies = context.lobby_manager->getAllLobbies();

        for (auto entity : to_destroy) {
            if (registry.hasComponent<NetworkIdentity>(entity)) {
                auto& netId = registry.getConstComponent<NetworkIdentity>(entity);

                // Pack guid into message
                network::message<network::GameEvents> msg;
                msg.header.id = network::GameEvents::S_ENTITY_DESTROY;
                msg << netId.guid;

                // Broadcast to all clients in games
                // Optimization: Could filter by lobby if entities had lobby ID,
                // but for now broadcast to all IN_GAME lobbies is safer/easier
                for (auto const& [lobbyId, lobby] : lobbies) {
                    if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                        continue;
                    }

                    for (const auto& client : lobby.getClients()) {
                        server->AddMessageToPlayer(network::GameEvents::S_ENTITY_DESTROY, client.id, netId.guid);
                    }
                }
            }
        }
    }
#endif

    // Perform local destruction
    for (auto entity : to_destroy) {
        registry.destroyEntity(entity);
    }
}
