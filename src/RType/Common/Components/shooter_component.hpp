#pragma once

#include "Components/StandardComponents.hpp"
#include "team_component.hpp"

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
