#pragma once

struct HealthComponent {
    static constexpr auto name = "HealthComponent";
    int max_hp;
    int current_hp;
    float last_damage_time = 0.0f;
    float invincibility_duration = 1.0f;
};
