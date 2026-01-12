#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "config.hpp"
#include "../Entities/Mobs/mob_spawner.hpp"
#include <map>
#include <string>
#include <vector>
#include <memory>

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

    // Seed pour le random (pas de static)
    unsigned int random_seed = 0;
    int random_state = 0;
};

class EnemySpawnSystem : public ISystem {
   public:
    EnemySpawnSystem();
    ~EnemySpawnSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void loadConfigs();
    void spawnEnemy(Registry& registry, system_context context, float x, float y, const std::string& enemy_type);
    void spawnBoss(Registry& registry, system_context context);
    void spawnObstacle(Registry& registry, system_context context, float x, float y);
    void handleObstacles(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp, float windowWidth,
                         float windowHeight);
    bool handleBossSpawn(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp);
    void spawnWave(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp, float windowWidth,
                   float windowHeight);

    int getRandomInt(EnemySpawnComponent& comp, int min, int max);
    float getRandomFloat(EnemySpawnComponent& comp, float min, float max);

    std::map<std::string, EntityConfig> _enemy_configs;
    EntityConfig _boss_config;
    GameConfig _game_config;
    std::vector<std::string> _enemy_types;
    std::map<std::string, std::unique_ptr<IMobSpawner>> _spawners;
    bool _configs_loaded = false;
};
