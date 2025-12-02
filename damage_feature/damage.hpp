#pragma one

namespace ECS {
struct DamageOnColision {
    int damage_value;
};

class Entity;
class Team;
class Damage {
   private:
    void on_collision_event(Entity attacker, Entity target);
    bool is_friend_fire(Team a, Team e);
    void apply_damage(Entity target, int damage);
};
}  // namespace ECS
