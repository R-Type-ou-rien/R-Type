#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/team_component.hpp"

#include "../Components/shooter_component.hpp"

class ShooterSystem : public ISystem {
   public:
    ShooterSystem() = default;
    ~ShooterSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    Velocity2D get_projectile_speed(ShooterComponent::ProjectileType type, TeamComponent::Team team);
      void create_projectile(Registry& registry, Entity owner_entity, ShooterComponent::ProjectileType type,
                        TeamComponent::Team team, transform_component_s pos, system_context context,
                        int projectile_damage, float projectile_scale);
      void create_projectile_with_pattern(Registry& registry, Entity owner_entity, ShooterComponent::ProjectileType type,
                                 TeamComponent::Team team, transform_component_s pos, system_context context,
                                 ShooterComponent::ShootPattern pattern, float target_x, float target_y,
                                 int projectile_damage, float projectile_scale);
    void create_charged_projectile(Registry& registry, Entity owner_entity, TeamComponent::Team team,
                     transform_component_s pos, system_context context, float charge_ratio);
      void create_pod_circular_laser(Registry& registry, Entity owner_entity, transform_component_s pos,
                              system_context context, int projectile_damage);
};
