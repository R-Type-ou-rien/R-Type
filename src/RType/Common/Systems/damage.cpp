#include "damage.hpp"

#include <set>
#include <vector>
#include "health.hpp"
#include "shooter.hpp"
#include "../Components/team_component.hpp"
#include "../Components/charged_shot.hpp"
#include "../Components/behavior_component.hpp"
#include "../Components/last_damage_dealer.hpp"

void Damage::update(Registry& registry, system_context context) {
    auto& attackers = registry.getEntities<DamageOnCollision>();
    std::set<Entity> damaged_this_frame;
    std::vector<Entity> attackers_to_destroy;

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

            // if (health.last_damage_time > 0) {
            //     continue;
            // }

            Entity dealer = attacker;
            if (registry.hasComponent<ProjectileComponent>(attacker)) {
                const auto& proj = registry.getConstComponent<ProjectileComponent>(attacker);
                dealer = static_cast<Entity>(proj.owner_id);
            }

            if (dealer != -1) {
                if (registry.hasComponent<LastDamageDealerComponent>(hit_id)) {
                    auto& last = registry.getComponent<LastDamageDealerComponent>(hit_id);
                    last.dealer_entity = dealer;
                } else {
                    registry.addComponent<LastDamageDealerComponent>(hit_id, {dealer});
                }
            }

            if (registry.hasComponent<BossComponent>(hit_id)) {
                auto& boss = registry.getComponent<BossComponent>(hit_id);
                boss.damage_flash_timer = boss.damage_flash_duration;
            }

            int damage_value = dmg.damage_value;
            if (registry.hasComponent<TagComponent>(attacker)) {
                const auto& tags = registry.getConstComponent<TagComponent>(attacker);
                for (const auto& tag : tags.tags) {
                    if (tag == "OBSTACLE") {
                        damage_value = 10;
                        break;
                    }
                }
            }

            damaged_this_frame.insert(hit_id);

            if (health.current_hp - damage_value <= 0) {
                health.current_hp = 0;
                std::cout << "[Damage] Entity " << hit_id << " killed by entity " << attacker << " (damage: " << damage_value
                          << ")" << std::endl;
            } else {
                health.current_hp -= damage_value;
                std::cout << "[Damage] Entity " << hit_id << " took " << damage_value
                          << " damage, HP remaining: " << health.current_hp << std::endl;
            }

            // health.last_damage_time = health.invincibility_duration;

            if (registry.hasComponent<TeamComponent>(attacker) && registry.hasComponent<TeamComponent>(hit_id)) {
                auto& teamA = registry.getConstComponent<TeamComponent>(attacker);
                auto& teamB = registry.getConstComponent<TeamComponent>(hit_id);
                if (teamA.team == TeamComponent::ENEMY && teamB.team == TeamComponent::ALLY) {
                    if (!registry.hasComponent<ProjectileComponent>(attacker)) {
                        attackers_to_destroy.push_back(attacker);
                        break;
                    }
                }
            }

            if (registry.hasComponent<ProjectileComponent>(attacker)) {
                if (registry.hasComponent<PenetratingProjectile>(attacker)) {
                    auto& penetrating = registry.getComponent<PenetratingProjectile>(attacker);
                    penetrating.current_penetrations++;

                    if (penetrating.current_penetrations >= penetrating.max_penetrations) {
                        attackers_to_destroy.push_back(attacker);
                        break;
                    }
                } else {
                    attackers_to_destroy.push_back(attacker);
                    break;
                }
            }
        }
    }

    for (auto attacker : attackers_to_destroy) {
        if (!registry.hasComponent<PendingDestruction>(attacker)) {
            registry.addComponent<PendingDestruction>(attacker, {});
        }
    }
}
