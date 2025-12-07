#pragma once

#include "../ecs/Components/Components.hpp"
#include "../ecs/Registry/registry.hpp"
#include "../ecs/System/ISystem.hpp"
#include "../team_component/team_component.hpp"

// namespace ECS {

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
    void init(Registry& reg) override {}

    void update(Registry& registry, float time);

   private:
    VelocityComponent get_projectile_speed(ShooterComponent::Projectile type, TeamComponent::Team team);
    void create_projectile(Registry& registry, ShooterComponent::Projectile type, TeamComponent::Team team,
                           transform_component_s pos, VelocityComponent velocity);
};

// }  // namespace ECS
