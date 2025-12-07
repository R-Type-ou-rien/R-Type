#include "box_collision/box_collision.hpp"
#include "damage_feature/damage.hpp"

void Damage::update(Registry& registry, system_context context) {
    auto& attackers = registry.getEntities<DamageOnColision>();

    for (auto attacker : attackers) {
        if (!registry.hasComponent<BoxCollisionComponent>(attacker))
            continue;
        auto& collider = registry.getComponent<BoxCollisionComponent>(attacker);

        if (collider.collision.tags.empty()) {
            continue;
        }

        for (auto& hit : collider.collision.tags) {
            Entity hit_id = hit;
            auto& dmg = registry.getComponent<DamageOnColision>(attacker);
            auto& health = registry.getComponent<HealthComponent>(hit_id);
            if (registry.hasComponent<TeamComponent>(attacker) && registry.hasComponent<TeamComponent>(hit_id)) {
                auto& teamA = registry.getComponent<TeamComponent>(attacker);
                auto& teamB = registry.getComponent<TeamComponent>(hit_id);
                if (teamA.team == teamB.team)
                    continue;
            }
            if (!registry.hasComponent<HealthComponent>(hit_id))
                continue;
            health.current_hp -= dmg.damage_value;
            std::cout << "Collision ! " << attacker << " a touche " << hit_id << " (-" << dmg.damage_value << " HP)"
                      << std::endl;
            if (registry.hasComponent<ProjectileComponent>(attacker)) {
                registry.destroyEntity(attacker);
                break;
            }
        }
    }
}
