#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "team_component.hpp"

struct ShooterComponent {
    enum ProjectileType { NORMAL, CHARG, RED, BLUE };
    ProjectileType type = NORMAL;
    bool trigger_pressed;
    bool is_shooting = false;
    double fire_rate = 0.f;
    double last_shot = 1000.0f;
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
    Velocity2D get_projectile_speed(ShooterComponent::ProjectileType type, TeamComponent::Team team);
    void create_projectile(Registry& registry, ShooterComponent::ProjectileType type, TeamComponent::Team team,
                           transform_component_s pos, system_context context);
};
