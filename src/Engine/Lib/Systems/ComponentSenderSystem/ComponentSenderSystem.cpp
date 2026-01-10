#include "ComponentSenderSystem.hpp"
#include "Context.hpp"
#include "ISystem.hpp"
#include "Network.hpp"
#include "registry.hpp"
#include "Components/NetworkComponents.hpp" // For NetworkIdentity and ComponentPacket
#include "ServerGameEngine.hpp" // For LobbyManager

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

    for (auto const& [lobbyId, lobby] : lobbies) {
        if (lobby.getState() != engine::core::Lobby::State::IN_GAME) {
            continue;
        }

        for (auto& [type, pool] : component_pools) {
            auto updated_entities = pool->getUpdatedEntities();
            if (updated_entities.empty()) {
                continue;
            }

            for (auto entity : updated_entities) {
                if (!reg.hasComponent<NetworkIdentity>(entity)) {
                    continue;
                }

                packet = pool->createPacket(entity, s_ctx);
                packet.entity_guid = reg.getConstComponent<NetworkIdentity>(entity).guid;
                
                server->AddMessageToLobby(network::GameEvents::S_SNAPSHOT, lobbyId, packet);
            }
            pool->clearUpdatedEntities();
        }
    }
}