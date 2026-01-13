#include "damage.hpp"

#include "health.hpp"
#include "shooter.hpp"
#include "../Components/team_component.hpp"
#include "../Components/charged_shot.hpp"
#include "../Components/ai_behavior_component.hpp"

void Damage::update(Registry& registry, system_context context) {
    auto& attackers = registry.getEntities<DamageOnCollision>();

    for (auto attacker : attackers) {
        if (!registry.hasComponent<BoxCollisionComponent>(attacker))
            continue;
        auto& collider = registry.getConstComponent<BoxCollisionComponent>(attacker);

        if (collider.collision.tags.empty()) {
            continue;
        }

        for (auto& hit : collider.collision.tags) {
            Entity hit_id = hit;
            auto& dmg = registry.getConstComponent<DamageOnCollision>(attacker);

            if (!registry.hasComponent<HealthComponent>(hit_id))
                continue;

            auto& health = registry.getComponent<HealthComponent>(hit_id);

            if (registry.hasComponent<TeamComponent>(attacker) && registry.hasComponent<TeamComponent>(hit_id)) {
                auto& teamA = registry.getConstComponent<TeamComponent>(attacker);
                auto& teamB = registry.getConstComponent<TeamComponent>(hit_id);
                if (teamA.team == teamB.team)
                    continue;
            }
            if (health.last_damage_time > 0) {
                continue;
            }

            // Trigger boss damage flash (visual feedback), even if this hit kills the boss.
            if (registry.hasComponent<BossComponent>(hit_id)) {
                auto& boss = registry.getComponent<BossComponent>(hit_id);
                boss.damage_flash_timer = boss.damage_flash_duration;
            }

            if (health.current_hp - dmg.damage_value <= 0) {
                health.current_hp = 0;
                std::cout << "[Damage] Entity " << hit_id << " killed by entity " << attacker << " (damage: " << dmg.damage_value << ")" << std::endl;
            } else {
                health.current_hp -= dmg.damage_value;
                health.last_damage_time = health.invincibility_duration;
                std::cout << "[Damage] Entity " << hit_id << " took " << dmg.damage_value << " damage, HP remaining: " << health.current_hp << std::endl;
            }

            if (registry.hasComponent<TeamComponent>(attacker) && registry.hasComponent<TeamComponent>(hit_id)) {
                auto& teamA = registry.getConstComponent<TeamComponent>(attacker);
                auto& teamB = registry.getConstComponent<TeamComponent>(hit_id);
                if (teamA.team == TeamComponent::ENEMY && teamB.team == TeamComponent::ALLY) {
                    if (!registry.hasComponent<ProjectileComponent>(attacker)) {
                        registry.destroyEntity(attacker);
                        break;
                    }
                }
            }

            if (registry.hasComponent<ProjectileComponent>(attacker)) {
                if (registry.hasComponent<PenetratingProjectile>(attacker)) {
                    auto& penetrating = registry.getComponent<PenetratingProjectile>(attacker);
                    penetrating.current_penetrations++;

                    if (penetrating.current_penetrations >= penetrating.max_penetrations) {
                        registry.destroyEntity(attacker);
                        break;
                    }
                } else {
                    registry.destroyEntity(attacker);
                    break;
                }
            }
        }
    }
}
