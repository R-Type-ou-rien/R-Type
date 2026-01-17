#include <vector>
#include <iostream>
#include "damage.hpp"
#include "Components/StandardComponents.hpp"
#include "score.hpp"
#include "behavior.hpp"

#include "../Components/last_damage_dealer.hpp"

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
            if (registry.hasComponent<BossComponent>(entity)) {
                continue;
            }
            dead_entities.push_back(entity);
        }
    }
    for (auto dead_entity : dead_entities) {
        if (registry.hasComponent<HealthComponent>(dead_entity)) {
            if (registry.hasComponent<ScoreValueComponent>(dead_entity)) {
                auto& score_value = registry.getConstComponent<ScoreValueComponent>(dead_entity);
                bool awarded_to_player = false;

                if (registry.hasComponent<LastDamageDealerComponent>(dead_entity)) {
                    const auto& last = registry.getConstComponent<LastDamageDealerComponent>(dead_entity);
                    const Entity dealer = last.dealer_entity;
                    if (dealer != -1 && registry.hasComponent<ScoreComponent>(dealer)) {
                        auto& dealer_score = registry.getComponent<ScoreComponent>(dealer);
                        dealer_score.current_score += score_value.value;
                        awarded_to_player = true;
                    }
                }

                if (!awarded_to_player) {
                    ScoreSystem::addScore(registry, score_value.value);
                }
            }
            registry.addComponent<PendingDestruction>(dead_entity, {});
        }
    }
}
