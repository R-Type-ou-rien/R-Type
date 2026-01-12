#pragma once

#include "ISystem.hpp"
#include "registry.hpp"
#include "../Components/config.hpp"
#include "../Components/spawn.hpp"
#include "../Components/scripted_spawn.hpp"
#include "../Entities/Mobs/mob_spawner.hpp"
#include <map>
#include <string>
#include <vector>
#include <memory>

class EnemySpawnSystem : public ISystem {
   public:
    EnemySpawnSystem() = default;
    ~EnemySpawnSystem() = default;
    void update(Registry& registry, system_context context) override;

   private:
    void loadConfigs(const std::string& enemies, const std::string& boss, const std::string& game);
    void spawnEnemy(Registry& registry, system_context context, float x, float y, const std::string& enemy_type);
    void spawnBoss(Registry& registry, system_context context);
    void spawnObstacle(Registry& registry, system_context context, float x, float y);
    void handleObstacles(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp, float windowWidth,
                         float windowHeight);
    bool handleBossSpawn(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp);
    void spawnWave(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp, float windowWidth,
                   float windowHeight);
    void loadScriptedSpawns(ScriptedSpawnComponent& scripted_spawn, const std::string& filename);
    void handleScriptedSpawns(Registry& registry, system_context context, ScriptedSpawnComponent& scripted_spawn,
                              float windowWidth, float windowHeight);

    int getRandomInt(EnemySpawnComponent& comp, int min, int max);
    float getRandomFloat(EnemySpawnComponent& comp, float min, float max);

    std::map<std::string, EntityConfig> _enemy_configs;
    EntityConfig _boss_config;
    GameConfig _game_config;
    std::vector<std::string> _enemy_types;
    std::map<std::string, std::unique_ptr<IMobSpawner>> _spawners;
    bool _configs_loaded = false;
};
