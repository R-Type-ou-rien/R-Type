#pragma once

// EnemySpawnComponent - Component definition only (no system)
// The EnemySpawnSystem is defined in Systems/spawn.hpp

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

    // Seed pour le random (pas de static)
    unsigned int random_seed = 0;
    int random_state = 0;
};
