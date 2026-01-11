#include "ComponentSenderSystem.hpp"
#include "Context.hpp"
#include "ISystem.hpp"
#include "Network.hpp"
#include "registry.hpp"
#include "Components/NetworkComponents.hpp"  // For NetworkIdentity and ComponentPacket
#include "ServerGameEngine.hpp"              // For LobbyManager
#include <iostream>
#include <variant>
#include "NetworkEngine/NetworkEngine.hpp"

void ComponentSenderSystem::update(Registry& reg, system_context ctx) {
    if (!ctx.lobby_manager) {
        return;
    }

    auto& lobbies = ctx.lobby_manager->getAllLobbies();
    if (lobbies.empty()) {
        return;
    }

    auto network_instance = ctx.network.getNetworkInstance();
    if (!std::holds_alternative<std::shared_ptr<network::Server>>(network_instance)) {
        return;
    }
    auto server = std::get<std::shared_ptr<network::Server>>(network_instance);

    ComponentPacket packet;
    SerializationContext s_ctx = {ctx.texture_manager};
    auto& component_pools = reg.getComponentPools();

    // Iterate over pools FIRST to capture updates once per frame
    for (auto& [type, pool] : component_pools) {
        auto updated_entities = pool->getUpdatedEntities();

        if (updated_entities.empty()) {
            continue;
        }

        // Create packets for all updated entities
        for (auto entity : updated_entities) {
            if (!reg.hasComponent<NetworkIdentity>(entity)) {
                continue;
            }

            packet = pool->createPacket(entity, s_ctx);
            packet.entity_guid = reg.getConstComponent<NetworkIdentity>(entity).guid;

            // Send to all IN_GAME lobbies
            for (auto const& [lobbyId, lobby] : lobbies) {
                if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
                    continue;
                }

                for (const auto& client : lobby.getClients()) {
                    server->AddMessageToPlayer(network::GameEvents::S_SNAPSHOT, client.id, packet);
                }
            }
        }
    }
}