#include <vector>
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
            // Ne pas détruire les boss immédiatement - ils ont une séquence de mort
            if (registry.hasComponent<BossComponent>(entity)) {
                // Le BossPatternSystem gère la mort du boss
                continue;
            }
            dead_entities.push_back(entity);
        }
    }
    for (auto dead_entity : dead_entities) {
        if (registry.hasComponent<HealthComponent>(dead_entity)) {
            // Ajouter le score si l'entité a une valeur de score
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
                        std::cout << "[HealthSystem] Entity " << dead_entity << " died, awarding score "
                                  << score_value.value << " to dealer " << dealer << std::endl;
                    }
                }

                if (!awarded_to_player) {
                    ScoreSystem::addScore(registry, score_value.value);
                    std::cout << "[HealthSystem] Entity " << dead_entity << " died, adding score: " << score_value.value << std::endl;
                }
            } else {
                std::cout << "[HealthSystem] Entity " << dead_entity << " died but has no ScoreValueComponent" << std::endl;
            }
            registry.addComponent<PendingDestruction>(dead_entity, {});
        }
    }
}
