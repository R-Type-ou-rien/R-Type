#include <vector>
#include "damage.hpp"
#include "Components/StandardComponents.hpp"
#include "score.hpp"

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
            // Ajouter le score si l'entité a une valeur de score
            if (registry.hasComponent<ScoreValueComponent>(dead_entity)) {
                auto& score_value = registry.getConstComponent<ScoreValueComponent>(dead_entity);
                ScoreSystem::addScore(registry, score_value.value);
            }
            registry.destroyEntity(dead_entity);
        }
    }
}
