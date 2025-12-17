#include <vector>
#include "damage.hpp"

#include "health.hpp"


void HealthSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<HealthComponent>();
    std::vector<Entity> dead_entities;

    for (auto entity : entities) {
        if (!registry.hasComponent<HealthComponent>(entity))
            continue;
        auto& health = registry.getComponent<HealthComponent>(entity);
        if (health.last_damage_time > 0) {
            health.last_damage_time -= context.dt;
        }
        if (health.current_hp <= 0) {
            dead_entities.push_back(entity);
        }
    }
    for (auto dead_entity : dead_entities) {
        if (registry.hasComponent<HealthComponent>(dead_entity)) {
            if (context.network_server.has_value()) {
                Server& server = context.network_server.value();
                auto& netIdent = registry.getComponent<NetworkIdentity>(dead_entity);
                if (netIdent.owner_user_id != 0) {  
                    std::cout << "Player " << netIdent.owner_user_id << " Died! Sending Game Over." << std::endl;
                    server.AddMessageToPlayer(GameEvents::S_GAME_OVER, netIdent.owner_user_id, 0);
                }
                
                auto& players = context.clients_id.value().get();
                uint32_t guid = static_cast<uint32_t>(netIdent.guid);
                for (auto& player_id : players) {
                    server.AddMessageToPlayer(GameEvents::S_PLAYER_DEATH, player_id, guid);
                }
            }
            registry.destroyEntity(dead_entity);
        }
    }
}
