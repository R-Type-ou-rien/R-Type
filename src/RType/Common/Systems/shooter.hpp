#pragma once

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/team_component.hpp"

struct ShooterComponent {
    static constexpr auto name = "ShooterComponent";
    enum ProjectileType { NORMAL, CHARG, RED, BLUE, POD_LASER };
    enum ShootPattern { STRAIGHT, AIM_PLAYER, SPREAD, CIRCULAR };
    ProjectileType type = NORMAL;
    ShootPattern pattern = STRAIGHT;  // Pattern par défaut
    bool is_shooting = false;
    bool trigger_pressed = false;
    double fire_rate = 0.f;
    double last_shot = 1000.0f;
    int projectile_damage = 30;  // Dégâts des projectiles de cette entité
    bool use_pod_laser = false;
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
    void create_projectile_with_pattern(Registry& registry, ShooterComponent::ProjectileType type,
                                        TeamComponent::Team team, transform_component_s pos, system_context context,
                                        ShooterComponent::ShootPattern pattern, float target_x, float target_y,
                                        int projectile_damage);
    void create_charged_projectile(Registry& registry, TeamComponent::Team team, transform_component_s pos,
                                   system_context context, float charge_ratio);
    void create_pod_circular_laser(Registry& registry, transform_component_s pos, system_context context,
                                   int projectile_damage);
};
