#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/config.hpp"
#include "../Components/scripted_spawn.hpp"
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
    unsigned int random_seed = 0;
    int random_state = 0;

    // Scripted spawn support
    bool use_scripted_spawns = true;
    std::string scripted_spawn_file = "src/RType/Common/content/config/level1_spawns.cfg";
};

class EnemySpawnSystem : public ISystem {
   public:
    EnemySpawnSystem();
    ~EnemySpawnSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void loadConfigs();
    void loadScriptedSpawns(const std::string& filepath);
    void spawnEnemy(Registry& registry, system_context context, float x, float y, const std::string& enemy_type);
    void spawnEnemyWithConfig(Registry& registry, system_context context, float x, float y,
                              const std::string& enemy_type, float custom_speed, int custom_hp);
    void spawnBoss(Registry& registry, system_context context);
    void spawnObstacle(Registry& registry, system_context context, float x, float y);
    void handleObstacles(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp, float windowWidth,
                         float windowHeight);
    bool handleBossSpawn(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp);
    void spawnWave(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp, float windowWidth,
                   float windowHeight);
    void handleScriptedSpawns(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp);
    void executeSpawnEvent(Registry& registry, system_context context, SpawnEvent& event);

    int getRandomInt(EnemySpawnComponent& comp, int min, int max);
    float getRandomFloat(EnemySpawnComponent& comp, float min, float max);

    std::map<std::string, EntityConfig> _enemy_configs;
    EntityConfig _boss_config;
    GameConfig _game_config;
    std::vector<std::string> _enemy_types;
    std::map<std::string, std::unique_ptr<IMobSpawner>> _spawners;
    bool _configs_loaded = false;

    // Scripted spawns
    std::vector<SpawnEvent> _scripted_spawns;
    bool _scripted_spawns_loaded = false;
};
