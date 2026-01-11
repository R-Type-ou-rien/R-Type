#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "team_component.hpp"
#include "shooter_component.hpp"

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
