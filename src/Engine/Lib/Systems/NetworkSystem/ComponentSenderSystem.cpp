#include "ComponentSenderSystem.hpp"
#include "registry.hpp"
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include "Components/NetworkComponents.hpp"
#include "Network/Network.hpp"
#include "Network/Server/Server.hpp"

void ComponentSenderSystem::update(Registry& registry, system_context context) {
    ComponentPacket packet;
    auto& component_pools = registry.getPools();

    if (!context.network_server.has_value() || !context.clients_id) {
        return;
    }
    Server& server = context.network_server.value();
    auto& players = context.clients_id.value().get();

    
    std::vector<uint32_t> new_ready_clients;
    for (auto& player : players) {
        if (server.IsClientReady(player)) {
            bool known = false;
            for (auto p : _known_ready_clients) {
                if (p == player) {
                    known = true;
                    break;
                }
            }
            if (!known) {
                new_ready_clients.push_back(player);
                _known_ready_clients.push_back(player);
                std::cout << "Client " << player << " is now ready. Sending full state." << std::endl;
            }
        }
    }

    
    if (!new_ready_clients.empty()) {
        auto& net_identities = registry.getPool<NetworkIdentity>().getIdList();

        for (auto& [type, pool] : component_pools) {
            auto entities = pool->getEntities();  
            for (auto entity : entities) {
                
                bool is_networked = false;
                
                
                
                if (!registry.hasComponent<NetworkIdentity>(entity))
                    continue;

                packet = pool->createPacket(entity);
                packet.entity_guid = registry.getComponentConst<NetworkIdentity>(entity).guid;

                for (auto client_id : new_ready_clients) {
                    server.AddMessageToPlayer(GameEvents::S_SNAPSHOT, client_id, packet);
                }
            }
        }
    }

    
    
    
    
    
    
    
    

    for (auto& [type, pool] : component_pools) {
        auto updated_components = pool->popUpdatedEntities();
        if (updated_components.empty())
            continue;
        for (auto entity : updated_components) {
            packet = pool->createPacket(entity);
            
            if (!registry.hasComponent<NetworkIdentity>(entity))
                continue;

            packet.entity_guid = registry.getComponentConst<NetworkIdentity>(entity).guid;
            for (auto& player : players) {
                if (!server.IsClientReady(player))
                    continue;
                server.AddMessageToPlayer(GameEvents::S_SNAPSHOT, player, packet);
            }
        }
    }
    return;
}
