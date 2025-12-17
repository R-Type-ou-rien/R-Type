#include <vector>
#include "damage.hpp"

#include "health.hpp"
// #include "collision.hpp"

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
            registry.destroyEntity(dead_entity);
        }
    }
}
