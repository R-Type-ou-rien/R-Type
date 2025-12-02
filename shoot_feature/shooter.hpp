#pragma once
#include <iostream>

namespace ECS {
class Registry;
class Type;
class Team;
class Position;

struct ShooterComponent {
    enum Projectile { NORMAL, CHARG, RED, BLUE };
    Projectile type;
    double fire_rate;
    double last_shot;
};

struct ProjectileComponent {
    int owner_id;
};

class ShooterSystem {
   public:
    ShooterSystem() = default;
    ~ShooterSystem() = default;

    void update(Registry& registry);

   private:
    void handle_shot(Registry& registry);      // + Entity e
    void check_pod_shoot(Registry& registry);  // + Entity e
    void create_projectile(Type type, Team team, Position pos);
};

}  // namespace ECS
