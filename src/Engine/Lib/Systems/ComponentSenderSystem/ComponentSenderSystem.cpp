#include "ComponentSenderSystem.hpp"
#include "Context.hpp"
#include "ISystem.hpp"
#include "Network.hpp"
#include "registry.hpp"
#include "Components/NetworkComponents.hpp"   // For NetworkIdentity and ComponentPacket
#include "Components/StandardComponents.hpp"  // For sprite2D_component_s, transform_component_s
#include "ServerGameEngine.hpp"               // For LobbyManager
#include <iostream>
#include <variant>
#include "NetworkEngine/NetworkEngine.hpp"
#include "ECS/Utils/Hash/Hash.hpp"  // For Hash::fnv1a

void ComponentSenderSystem::update(Registry& reg, system_context ctx) {
    if (!ctx.lobby_manager) {
        return;
    }

    if (!ctx.networked_component_types) {
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

    // Debug: Print all registered networked component type hashes
    static bool printed_hashes = false;
    if (!printed_hashes) {
        std::cout << "[ComponentSenderSystem] Registered networked component types: ";
        for (auto h : *ctx.networked_component_types) {
            std::cout << h << " ";
        }
        std::cout << std::endl;

        // Print what hash Sprite2DComponent would be
        std::cout << "[ComponentSenderSystem] Sprite2DComponent hash: " << Hash::fnv1a("Sprite2DComponent")
                  << std::endl;

        printed_hashes = true;
    }

    // Iterate over pools to send only updated (dirty) components
    for (auto& [type, pool] : component_pools) {
        // Skip components that are not registered for network replication
        uint32_t typeHash = pool->getTypeHash();
        if (ctx.networked_component_types->find(typeHash) == ctx.networked_component_types->end()) {
            continue;  // This component type is not networked, skip it
        }

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