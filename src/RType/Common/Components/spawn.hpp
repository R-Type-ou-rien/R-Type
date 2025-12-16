#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct EnemySpawnComponent {
    float spawn_timer = 0.0f;
    float spawn_interval = 3.0f;
    float total_time = 0.0f;
    bool boss_spawned = false;
    int wave_count = 0;
    int enemies_per_wave = 3;
    bool is_active = true;
};

class EnemySpawnSystem : public ISystem {
   public:
    EnemySpawnSystem() = default;
    ~EnemySpawnSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void spawnEnemy(Registry& registry, system_context context, float x, float y, bool sine_pattern);
    void spawnBoss(Registry& registry, system_context context);
};
