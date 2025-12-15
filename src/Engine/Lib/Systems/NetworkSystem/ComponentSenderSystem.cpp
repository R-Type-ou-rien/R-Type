#include "ComponentSenderSystem.hpp"
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include "Components/NetworkComponents.hpp"

void ComponentSenderSystem::update(Registry& registry, system_context context)
{
    ComponentPacket packet;
    auto& component_pools = registry.getPools();

    if (!context.network_server.has_value()) {
        throw std::logic_error("The server network class is not initalized in the given context");
    }

    for (auto& [type, pool] : component_pools) {
        auto updated_components = pool->popUpdatedEntities();
        if (updated_components.empty())
            continue;
        for (auto entity : updated_components) {
            packet = pool->createPacket(entity);
            packet.entity_guid = registry.getComponentConst<NetworkIdentity>(entity).guid;
        }
    }
    return;
}
