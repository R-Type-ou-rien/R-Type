#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "team_component.hpp"


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
    ShooterSystem() = default;
    ~ShooterSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    VelocityComponent get_projectile_speed(ShooterComponent::Projectile type, TeamComponent::Team team);
    void create_projectile(Registry& registry, ShooterComponent::Projectile type, TeamComponent::Team team,
                           transform_component_s pos, VelocityComponent velocity);
};
