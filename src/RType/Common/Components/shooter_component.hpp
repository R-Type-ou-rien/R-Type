#pragma once

#include "Components/StandardComponents.hpp"
#include "team_component.hpp"

struct ShooterComponent {
    static constexpr auto name = "ShooterComponent";
    enum ProjectileType { NORMAL, CHARG, RED, BLUE, POD_LASER };
    enum ShootPattern { STRAIGHT, AIM_PLAYER, SPREAD, CIRCULAR };
    ProjectileType type = NORMAL;
    ShootPattern pattern = STRAIGHT;
    bool is_shooting = false;
    bool trigger_pressed = false;
    double fire_rate = 0.f;
    double last_shot = 1000.0f;
    int projectile_damage = 30;
    float projectile_scale = 3.0f;
    bool use_pod_laser = false;
};

struct ProjectileComponent {
    static constexpr auto name = "ProjectileComponent";
    int owner_id;
};
