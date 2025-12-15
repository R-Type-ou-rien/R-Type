#include "NetworkSystem.hpp"
#include <iostream>
#include <iterator>
#include <ostream>
#include "Components/NetworkComponents.hpp"

void NetworkSystem::update(Registry& registry, system_context context)
{
    ComponentPacket packet;
    auto& component_pools = registry.getPools();

    std::cout << "ENTER UPDATE NETWORK WITH " << component_pools.size() << std::endl;
    for (auto& [type, pool] : component_pools) {
        std::cout << "A" << std::endl;
        auto updated_components = pool->popUpdatedEntities();
        if (updated_components.empty())
            continue;
        std::cout << "B" << std::endl;
        for (auto entity : updated_components) {
            std::cout << "C" << std::endl;
            packet = pool->createPacket(entity);
            std::cout << "D" << std::endl;
            packet.entity_guid = registry.getComponentConst<NetworkIdentity>(entity).guid;
            std::cout << "GUID of entity " << entity << ": " << packet.entity_guid << std::endl;
            std::cout << " component type:" << packet.component_type << std::endl;
        }
    }
    return;
}
