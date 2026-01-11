#include "damage.hpp"

#include "Components/StandardComponents.hpp"
#include "CollisionSystem.hpp"
#include "health.hpp"
#include "shooter.hpp"
#include "../Components/team_component.hpp"
#include "../Components/charged_shot.hpp"
#include <set>
#include <vector>

void Damage::update(Registry& registry, system_context context) {
    auto& attackers = registry.getEntities<DamageOnCollision>();

    // Track entities that have already been damaged this frame
    std::set<Entity> damaged_this_frame;
    // Track attackers to destroy after processing
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

            // Check invincibility from previous frames
            if (health.last_damage_time > 0) {
                continue;
            }

            // Check if already damaged this frame by another attacker
            if (damaged_this_frame.count(hit_id) > 0) {
                continue;
            }

            int damage_value = dmg.damage_value;

            if (registry.hasComponent<TagComponent>(attacker)) {
                auto& tags = registry.getConstComponent<TagComponent>(attacker);
                for (const auto& tag : tags.tags) {
                    if (tag == "OBSTACLE") {
                        damage_value = 10;
                    }
                }
            }

            // Mark as damaged this frame BEFORE applying damage
            damaged_this_frame.insert(hit_id);

            // Always set invincibility after taking damage, even on death
            health.last_damage_time = health.invincibility_duration;

            if (health.current_hp - damage_value <= 0) {
                health.current_hp = 0;
            } else {
                health.current_hp -= damage_value;
            }

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

    // Destroy attackers after processing to avoid iterator invalidation
    for (auto attacker : attackers_to_destroy) {
        registry.destroyEntity(attacker);
    }
}
