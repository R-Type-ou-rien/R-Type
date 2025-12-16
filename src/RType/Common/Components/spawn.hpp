#pragma once

#include "ISystem.hpp"
#include "registry.hpp"

struct SpawnComponent {
    float spawn_timer = 0.0f;
    float spawn_interval = 3.0f;
    int wave_count = 0;
    int enemies_per_wave = 3;
    bool is_active = true;
};

class SpawnSystem : public ISystem {
   public:
    SpawnSystem() = default;
    ~SpawnSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void spawnEnemy(Registry& registry, system_context context, float x, float y, bool sine_pattern);
};
