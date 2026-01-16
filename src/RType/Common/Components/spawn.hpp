#pragma once

#include <string>

struct EnemySpawnComponent {
    static constexpr auto name = "EnemySpawnComponent";
    float spawn_timer = 0.0f;
    float spawn_interval = 2.0f;
    float total_time = 0.0f;
    bool boss_spawned = false;
    bool boss_arrived = false;
    float boss_intro_timer = 0.0f;
    int wave_count = 0;
    bool is_active = true;
    bool use_scripted_spawns = true;

    std::string enemies_config_path;
    std::string boss_config_path;
    std::string boss_section = "DEFAULT";
    std::string game_config_path;

    unsigned int random_seed = 0;
    int random_state = 0;
};
