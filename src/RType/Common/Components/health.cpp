#include <vector>
#include "damage.hpp"
#include "src/ecs/common/box_collision/box_collision.hpp"

#include "health.hpp"
// #include "collision.hpp"

void HealthSystem::update(Registry& registry, system_context context) {
    auto& entities = registry.getEntities<HealthComponent>();
    std::vector<Entity> dead_entities;

    
    for (auto entity : entities) {
        auto& health = registry.getComponent<HealthComponent>(entity);
        auto& collision = registry.getComponent<BoxCollisionComponent>(entity);
        for (auto& hit : collision.collision.tags) {
            Entity hit_id = hit;
            auto& dmg = registry.getComponent<DamageOnCollision>(hit_id);
            if (health.current_hp - dmg.damage_value <= 0) {
                health.current_hp = 0;
            } else {
                health.current_hp -= dmg.damage_value;
            }
        }
        if (health.current_hp <= 0) {
            dead_entities.push_back(entity);
        }
    }
    for (auto dead_entity : dead_entities) {
        registry.destroyEntity(dead_entity);
    }
}
