#include "spawn.hpp"
#include <ctime>
#include <string>
#include "Components/StandardComponents.hpp"
#include "../Entities/Mobs/all_mobs.hpp"

const float WORLD_WIDTH = 1920.0f;
const float WORLD_HEIGHT = 1080.0f;

EnemySpawnSystem::EnemySpawnSystem() {
    loadConfigs();
}

void EnemySpawnSystem::loadConfigs() {
    if (_configs_loaded)
        return;

    _enemy_configs =
        ConfigLoader::loadEnemiesConfig("content/config/enemies.cfg", ConfigLoader::getRequiredEnemyFields());
    _boss_config = ConfigLoader::loadEntityConfig("content/config/boss.cfg", ConfigLoader::getRequiredBossFields());
    _game_config = ConfigLoader::loadGameConfig("content/config/game.cfg", ConfigLoader::getRequiredGameFields());

    for (const auto& pair : _enemy_configs) {
        _enemy_types.push_back(pair.first);
    }

    _spawners = MobSpawnerFactory::createSpawners();
    _configs_loaded = true;
}

int EnemySpawnSystem::getRandomInt(EnemySpawnComponent& comp, int min, int max) {
    comp.random_state = (comp.random_state * 1103515245 + 12345) & 0x7fffffff;
    return min + (comp.random_state % (max - min + 1));
}

float EnemySpawnSystem::getRandomFloat(EnemySpawnComponent& comp, float min, float max) {
    comp.random_state = (comp.random_state * 1103515245 + 12345) & 0x7fffffff;
    float normalized = static_cast<float>(comp.random_state) / static_cast<float>(0x7fffffff);
    return min + normalized * (max - min);
}

void EnemySpawnSystem::update(Registry& registry, system_context context) {
    auto& spawners = registry.getEntities<EnemySpawnComponent>();
    float windowWidth = WORLD_WIDTH;
    float windowHeight = WORLD_HEIGHT;

    // Nettoyage des entités hors écran
    auto& entities = registry.getEntities<transform_component_s>();
    for (auto entity : entities) {
        if (!registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        bool is_enemy = false;
        for (const auto& tag : tags.tags) {
            if (tag == "AI" || tag == "ENEMY_PROJECTILE" || tag == "OBSTACLE") {
                is_enemy = true;
                break;
            }
        }
        if (!is_enemy)
            continue;
        auto& transform = registry.getConstComponent<transform_component_s>(entity);
        if (transform.x < -300.0f) {
            registry.destroyEntity(entity);
        }
    }

    for (auto spawner : spawners) {
        auto& spawn_comp = registry.getComponent<EnemySpawnComponent>(spawner);

        if (spawn_comp.random_seed == 0) {
            spawn_comp.random_seed = static_cast<unsigned int>(std::time(nullptr));
            spawn_comp.random_state = spawn_comp.random_seed;
        }

        spawn_comp.total_time += context.dt;

        // Obstacles avant le boss
        handleObstacles(registry, context, spawn_comp, windowWidth, windowHeight);

        // Gestion du boss
        if (handleBossSpawn(registry, context, spawn_comp))
            continue;

        if (!spawn_comp.is_active)
            continue;

        // Spawn des vagues normales
        spawn_comp.spawn_timer += context.dt;
        if (spawn_comp.spawn_timer >= _game_config.wave_interval.value()) {
            spawn_comp.spawn_timer = 0.0f;
            spawnWave(registry, context, spawn_comp, windowWidth, windowHeight);
        }
    }
}

void EnemySpawnSystem::handleObstacles(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp,
                                       float windowWidth, float windowHeight) {
    float obstacle_start = _game_config.obstacle_start_time.value();
    float obstacle_end = _game_config.obstacle_end_time.value();

    if (spawn_comp.total_time >= obstacle_start && spawn_comp.total_time < obstacle_end && !spawn_comp.boss_spawned) {
        float obstacle_interval = 3.0f;
        int obstacle_count = static_cast<int>((spawn_comp.total_time - obstacle_start) / obstacle_interval);

        if (spawn_comp.wave_count < obstacle_count + 100) {
            float y_pos = getRandomFloat(spawn_comp, 100.0f, windowHeight - 150.0f);
            spawnObstacle(registry, context, windowWidth + 100.0f, y_pos);
            spawn_comp.wave_count = obstacle_count + 100;
        }
    }
}

bool EnemySpawnSystem::handleBossSpawn(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp) {
    if (spawn_comp.total_time >= _game_config.boss_spawn_time.value() && !spawn_comp.boss_spawned) {
        auto& enemies = registry.getEntities<TagComponent>();
        int enemy_count = 0;
        for (auto entity : enemies) {
            auto& tags = registry.getConstComponent<TagComponent>(entity);
            bool is_ai = false, is_boss = false;
            for (const auto& tag : tags.tags) {
                if (tag == "AI")
                    is_ai = true;
                if (tag == "BOSS")
                    is_boss = true;
            }
            if (is_ai && !is_boss)
                enemy_count++;
        }

        if (enemy_count == 0) {
            spawn_comp.is_active = false;
            spawn_comp.boss_spawned = true;
            spawn_comp.boss_intro_timer = 0.0f;

            // Stop background
            auto& backgrounds = registry.getEntities<BackgroundComponent>();
            for (auto bg_entity : backgrounds) {
                auto& bg = registry.getComponent<BackgroundComponent>(bg_entity);
                bg.scroll_speed = 0.0f;
            }
        }
    }

    if (spawn_comp.boss_spawned && !spawn_comp.boss_arrived) {
        spawn_comp.boss_intro_timer += context.dt;
        if (spawn_comp.boss_intro_timer >= _game_config.boss_intro_delay.value()) {
            spawnBoss(registry, context);
            spawn_comp.boss_arrived = true;
        }
        return true;
    }
    return false;
}

void EnemySpawnSystem::spawnWave(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp,
                                 float windowWidth, float windowHeight) {
    int enemies_to_spawn =
        getRandomInt(spawn_comp, _game_config.min_enemies_per_wave.value(), _game_config.max_enemies_per_wave.value());
    bool spawn_as_group = (getRandomInt(spawn_comp, 0, 2) == 0);

    if (spawn_as_group && !_enemy_types.empty()) {
        float base_y = getRandomFloat(spawn_comp, 150.0f, windowHeight - 250.0f);
        int type_idx = getRandomInt(spawn_comp, 0, static_cast<int>(_enemy_types.size()) - 1);
        std::string group_type = _enemy_types[type_idx];

        for (int i = 0; i < enemies_to_spawn; i++) {
            float y_offset = i * 80.0f;
            float y_position = base_y + y_offset;
            if (y_position > windowHeight - 100.0f)
                y_position = windowHeight - 100.0f;
            spawnEnemy(registry, context, windowWidth + 50.0f + i * 60.0f, y_position, group_type);
        }
    } else if (!_enemy_types.empty()) {
        for (int i = 0; i < enemies_to_spawn; i++) {
            float y_position = getRandomFloat(spawn_comp, 100.0f, windowHeight - 100.0f);
            int type_idx = getRandomInt(spawn_comp, 0, static_cast<int>(_enemy_types.size()) - 1);
            spawnEnemy(registry, context, windowWidth + 50.0f, y_position, _enemy_types[type_idx]);
        }
    }
}

void EnemySpawnSystem::spawnEnemy(Registry& registry, system_context context, float x, float y,
                                  const std::string& enemy_type) {
    auto it = _enemy_configs.find(enemy_type);
    if (it == _enemy_configs.end())
        return;

    auto spawner_it = _spawners.find(enemy_type);
    if (spawner_it != _spawners.end()) {
        spawner_it->second->spawn(registry, context, x, y, it->second);
    }
}

void EnemySpawnSystem::spawnBoss(Registry& registry, system_context context) {
    auto spawner_it = _spawners.find("BOSS");
    if (spawner_it != _spawners.end()) {
        spawner_it->second->spawn(registry, context, 0, 0, _boss_config);
    }
}

void EnemySpawnSystem::spawnObstacle(Registry& registry, system_context context, float x, float y) {
    auto spawner_it = _spawners.find("OBSTACLE");
    if (spawner_it != _spawners.end()) {
        EntityConfig dummy;
        spawner_it->second->spawn(registry, context, x, y, dummy);
    }
}
