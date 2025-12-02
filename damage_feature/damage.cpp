#pragma one

#include "damage_feature/damage.hpp"

namespace ECS {
void Damage::on_collision_event(Entity attacker, Entity target) {}

bool Damage::is_friend_fire(Team a, Team e) {
    return false;
}
void Damage::apply_damage(Entity target, int damage) {}
}  // namespace ECS
