#pragma once

#include <string>

struct ChargedShotComponent {
    static constexpr auto name = "ChargedShotComponent";

    bool is_charging = false;
    float charge_time = 0.0f;
    float min_charge_time = 0.5f;
    float max_charge_time = 2.0f;
    float medium_charge = 1.0f;
};

struct PenetratingProjectile {
    static constexpr auto name = "PenetratingProjectile";

    int max_penetrations = 1;
    int current_penetrations = 0;
};

struct ProjectileConfigComponent {
    static constexpr auto name = "ProjectileConfigComponent";

    std::string projectile_sprite = "src/RType/Common/content/sprites/r-typesheet1.gif";
    int projectile_sprite_x = 232;
    int projectile_sprite_y = 103;
    int projectile_sprite_w = 32;
    int projectile_sprite_h = 14;

    std::string charged_sprite = "src/RType/Common/content/sprites/r-typesheet1.gif";
    int charged_sprite_x = 263;
    int charged_sprite_y = 120;
    int charged_sprite_w = 64;
    int charged_sprite_h = 56;
};
