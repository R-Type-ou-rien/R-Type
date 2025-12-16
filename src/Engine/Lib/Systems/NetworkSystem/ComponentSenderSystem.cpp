#include "ComponentSenderSystem.hpp"
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include "Components/NetworkComponents.hpp"
#include "Network/Network.hpp"
#include "Network/Server/Server.hpp"

void ComponentSenderSystem::update(Registry& registry, system_context context)
{
    ComponentPacket packet;
    auto& component_pools = registry.getPools();

    if (!context.network_server.has_value() || !context.clients_id) {
        return;
    }
    Server& server = context.network_server.value();
    auto& players = context.clients_id.value().get();

    for (auto& [type, pool] : component_pools) {
        auto updated_components = pool->popUpdatedEntities();
        if (updated_components.empty())
            continue;
        for (auto entity : updated_components) {
            packet = pool->createPacket(entity);
            packet.entity_guid = registry.getComponentConst<NetworkIdentity>(entity).guid;
            for (auto& player : players)
                server.AddMessageToPlayer(GameEvents::S_SNAPSHOT, player, packet);
        }
    }
    return;
}
