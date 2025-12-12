#include <vector>

#include "health.hpp"

void HealthSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<HealthComponent>();
    std::vector<Entity> dead_entities;

    for (auto entity : entities) {
        auto& health = registry.getComponent<HealthComponent>(entity);
        if (health.current_hp <= 0) {
            dead_entities.push_back(entity);
        }
    }
    for (auto dead_entity : dead_entities) {
        registry.destroyEntity(dead_entity);
    }
}
