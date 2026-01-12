#include "ai_behavior.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/team_component.hpp"
#include "shooter.hpp"
#include "health.hpp"
#include <cmath>

void AIBehaviorSystem::update(Registry& registry, system_context context) {
    // Trouver le joueur
    Entity player_entity = -1;
    auto& teams = registry.getEntities<TeamComponent>();
    for (auto entity : teams) {
        auto& team = registry.getConstComponent<TeamComponent>(entity);
        if (team.team == TeamComponent::ALLY) {
            if (registry.hasComponent<TagComponent>(entity)) {
                auto& tags = registry.getConstComponent<TagComponent>(entity);
                for (const auto& tag : tags.tags) {
                    if (tag == "PLAYER") {
                        player_entity = entity;
                        break;
                    }
                }
            }
        }
        if (player_entity != -1)
            break;
    }

    if (player_entity == -1)
        return;

    // Mettre à jour les comportements des ennemis
    auto& behaviors = registry.getEntities<AIBehaviorComponent>();
    for (auto entity : behaviors) {
        auto& behavior = registry.getComponent<AIBehaviorComponent>(entity);

        if (behavior.follow_player) {
            updateFollowPlayer(registry, context, entity, player_entity);
        }

        if (behavior.shoot_at_player) {
            updateShootAtPlayer(registry, entity, player_entity);
        }
    }

    // Gérer le boss qui arrive et ses patterns d'attaque
    auto& bosses = registry.getEntities<BossComponent>();
    for (auto boss_entity : bosses) {
        auto& boss = registry.getComponent<BossComponent>(boss_entity);

        // Phase d'arrivée
        if (!boss.has_arrived && registry.hasComponent<transform_component_s>(boss_entity)) {
            auto& transform = registry.getComponent<transform_component_s>(boss_entity);

            if (transform.x <= boss.target_x) {
                transform.x = boss.target_x;
                boss.has_arrived = true;

                if (registry.hasComponent<Velocity2D>(boss_entity)) {
                    auto& vel = registry.getComponent<Velocity2D>(boss_entity);
                    vel.vx = 0.0f;
                    vel.vy = 0.0f;
                }
            }
        }
        
        // Gestion des phases du boss
        if (boss.has_arrived && registry.hasComponent<HealthComponent>(boss_entity)) {
            auto& health = registry.getConstComponent<HealthComponent>(boss_entity);
            float health_percent = static_cast<float>(health.current_hp) / static_cast<float>(health.max_hp);
            
            int new_phase = boss.current_phase;
            if (health_percent <= 0.33f && boss.current_phase < 3) {
                new_phase = 3;  // Phase finale
            } else if (health_percent <= 0.66f && boss.current_phase < 2) {
                new_phase = 2;  // Phase intermédiaire
            }
            
            // Transition de phase
            if (new_phase != boss.current_phase) {
                boss.current_phase = new_phase;
                
                // Augmenter le fire rate en fonction de la phase
                if (registry.hasComponent<ShooterComponent>(boss_entity)) {
                    auto& shooter = registry.getComponent<ShooterComponent>(boss_entity);
                    shooter.fire_rate = 0.3f / boss.current_phase;  // Plus rapide à chaque phase
                }
            }
            
            // Pattern d'attaque
            boss.attack_pattern_timer += context.dt;
            if (boss.attack_pattern_timer >= boss.attack_pattern_interval) {
                boss.attack_pattern_timer = 0.0f;
                boss.current_attack_pattern = (boss.current_attack_pattern + 1) % 3;
                
                // Changer le pattern de tir
                if (registry.hasComponent<ShooterComponent>(boss_entity)) {
                    auto& shooter = registry.getComponent<ShooterComponent>(boss_entity);
                    switch (boss.current_attack_pattern) {
                        case 0:
                            shooter.pattern = ShooterComponent::STRAIGHT;
                            break;
                        case 1:
                            shooter.pattern = ShooterComponent::AIM_PLAYER;
                            break;
                        case 2:
                            shooter.pattern = ShooterComponent::SPREAD;
                            break;
                    }
                }
            }
        }
    }
}

void AIBehaviorSystem::updateFollowPlayer(Registry& registry, system_context context, Entity enemy, Entity player) {
    if (!registry.hasComponent<transform_component_s>(enemy) || !registry.hasComponent<transform_component_s>(player)) {
        return;
    }

    auto& enemy_transform = registry.getComponent<transform_component_s>(enemy);
    auto& player_transform = registry.getConstComponent<transform_component_s>(player);
    auto& behavior = registry.getConstComponent<AIBehaviorComponent>(enemy);

    float dx = player_transform.x - enemy_transform.x;
    float dy = player_transform.y - enemy_transform.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 5.0f) {
        if (registry.hasComponent<Velocity2D>(enemy)) {
            auto& vel = registry.getComponent<Velocity2D>(enemy);
            vel.vx = (dx / distance) * behavior.follow_speed;
            vel.vy = (dy / distance) * behavior.follow_speed;
        }
    }
}

void AIBehaviorSystem::updateShootAtPlayer(Registry& registry, Entity enemy, Entity player) {
    // La direction du tir sera calculée dans le ShooterSystem
    // On stocke juste la position du joueur pour référence
}

void BoundsSystem::update(Registry& registry, system_context context) {
    auto& bounds_entities = registry.getEntities<WorldBoundsComponent>();
    if (bounds_entities.empty())
        return;

    auto& bounds = registry.getConstComponent<WorldBoundsComponent>(bounds_entities[0]);

    auto& players = registry.getEntities<TeamComponent>();
    for (auto entity : players) {
        auto& team = registry.getConstComponent<TeamComponent>(entity);
        if (team.team != TeamComponent::ALLY)
            continue;

        if (!registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = registry.getConstComponent<TagComponent>(entity);

        bool is_player = false;
        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                is_player = true;
                break;
            }
        }

        if (!is_player)
            continue;

        if (registry.hasComponent<transform_component_s>(entity)) {
            auto& transform = registry.getComponent<transform_component_s>(entity);

            float sprite_w = 33.0f;
            float sprite_h = 17.0f;
            float scale_x = transform.scale_x;
            float scale_y = transform.scale_y;

            if (registry.hasComponent<sprite2D_component_s>(entity)) {
                auto& sprite = registry.getConstComponent<sprite2D_component_s>(entity);
                sprite_w = sprite.dimension.width;
                sprite_h = sprite.dimension.height;
            }

            float actual_width = sprite_w * scale_x;
            float actual_height = sprite_h * scale_y;

            if (transform.x < bounds.min_x)
                transform.x = bounds.min_x;
            if (transform.x > bounds.max_x - actual_width)
                transform.x = bounds.max_x - actual_width;
            if (transform.y < bounds.min_y)
                transform.y = bounds.min_y;
            if (transform.y > bounds.max_y - actual_height)
                transform.y = bounds.max_y - actual_height;
        }
    }
}
