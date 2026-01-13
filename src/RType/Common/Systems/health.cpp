#include <vector>
#include "damage.hpp"
#include "Components/StandardComponents.hpp"
#include "score.hpp"
#include "ai_behavior.hpp"

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
            std::cout << "[SCORE DEBUG] Entity " << dead_entity << " is dead, checking score attribution..." << std::endl;
            
            // Ajouter le score si l'entité a une valeur de score
            if (registry.hasComponent<ScoreValueComponent>(dead_entity)) {
                auto& score_value = registry.getConstComponent<ScoreValueComponent>(dead_entity);
                bool awarded_to_player = false;
                
                std::cout << "[SCORE DEBUG] Entity " << dead_entity << " has ScoreValueComponent value=" << score_value.value << std::endl;

                if (registry.hasComponent<LastDamageDealerComponent>(dead_entity)) {
                    const auto& last = registry.getConstComponent<LastDamageDealerComponent>(dead_entity);
                    const Entity dealer = last.dealer_entity;
                    std::cout << "[SCORE DEBUG] LastDamageDealer for entity " << dead_entity << " is dealer=" << dealer << std::endl;
                    
                    if (dealer != static_cast<Entity>(-1) && registry.hasComponent<ScoreComponent>(dealer)) {
                        auto& dealer_score = registry.getComponent<ScoreComponent>(dealer);
                        std::cout << "[SCORE DEBUG] Dealer " << dealer << " has ScoreComponent, current_score=" << dealer_score.current_score << std::endl;
                        dealer_score.current_score += score_value.value;
                        awarded_to_player = true;
                        std::cout << "[HealthSystem] Entity " << dead_entity << " died, awarding score "
                                  << score_value.value << " to dealer " << dealer 
                                  << ", new_score=" << dealer_score.current_score << std::endl;
                    } else {
                        std::cerr << "[SCORE DEBUG] ERROR: Dealer " << dealer << " has NO ScoreComponent!" << std::endl;
                    }
                } else {
                    std::cerr << "[SCORE DEBUG] ERROR: Entity " << dead_entity << " has NO LastDamageDealerComponent!" << std::endl;
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
