<<<<<<< HEAD:src/RType/Common/Components/health.cpp
#include "health.hpp"
=======
#include <vector>

#include "ecs/common/health_feature/health.hpp"

#include "ecs/common/Components/Components.hpp"
>>>>>>> 2e0d1a29fa2d0e6b3713286aabdb39628515dfd4:src/ecs/common/health_feature/health.cpp

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
