#pragma once

#include "../../common/Components/Components.hpp"
#include "../team_component/team_component.hpp"
#include "ecs/common/ISystem.hpp"
#include "ecs/common/Registry/registry.hpp"

struct VelocityComponent {
    double vx;
    double vy;
};

struct ShooterComponent {
    enum Projectile { NORMAL, CHARG, RED, BLUE };
    Projectile type;
    double fire_rate;
    double last_shot;
};

struct ProjectileComponent {
    int owner_id;
};

class ShooterSystem : public ISystem {
   public:
    void update(Registry& registry, float time);

   private:
    VelocityComponent get_projectile_speed(ShooterComponent::Projectile type, TeamComponent::Team team);
    void create_projectile(Registry& registry, ShooterComponent::Projectile type, TeamComponent::Team team,
                           transform_component_s pos, VelocityComponent velocity);
};
