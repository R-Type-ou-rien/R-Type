#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/team_component.hpp"

struct ShooterComponent {
    static constexpr auto name = "ShooterComponent";
    enum ProjectileType { NORMAL, CHARG, RED, BLUE };
    ProjectileType type = NORMAL;
    bool is_shooting = false;
    bool trigger_pressed = false;
    double fire_rate = 0.f;
    double last_shot = 1000.0f;
};

struct ProjectileComponent {
    static constexpr auto name = "ProjectileComponent";
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
    void create_charged_projectile(Registry& registry, TeamComponent::Team team, transform_component_s pos,
                                   system_context context, float charge_ratio);
};
