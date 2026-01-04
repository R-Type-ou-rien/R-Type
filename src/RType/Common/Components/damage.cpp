#include "damage.hpp"

#include "CollisionSystem.hpp"
#include "health.hpp"
#include "shooter.hpp"
#include "team_component.hpp"

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

            if (health.current_hp - dmg.damage_value <= 0) {
                health.current_hp = 0;
            } else {
                health.current_hp -= dmg.damage_value;
                health.last_damage_time = health.invincibility_duration;
            }

            if (registry.hasComponent<ProjectileComponent>(attacker)) {
                registry.destroyEntity(attacker);
                break;
            }
        }
    }
}
