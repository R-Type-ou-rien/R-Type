#include "spawn.hpp"
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "Components/StandardComponents.hpp"
#include "../Entities/Mobs/all_mobs.hpp"
#include "../Components/game_timer.hpp"

const float WORLD_WIDTH = 1920.0f;
const float WORLD_HEIGHT = 1080.0f;

EnemySpawnSystem::EnemySpawnSystem() {
    loadConfigs();
    loadScriptedSpawns("src/RType/Common/content/config/level1_spawns.cfg");
}

void EnemySpawnSystem::loadConfigs() {
    if (_configs_loaded)
        return;

    _enemy_configs = ConfigLoader::loadEnemiesConfig("src/RType/Common/content/config/enemies.cfg",
                                                     ConfigLoader::getRequiredEnemyFields());
    _boss_config = ConfigLoader::loadEntityConfig("src/RType/Common/content/config/boss.cfg",
                                                  ConfigLoader::getRequiredBossFields());
    _game_config =
        ConfigLoader::loadGameConfig("src/RType/Common/content/config/game.cfg", ConfigLoader::getRequiredGameFields());

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

    // Mettre à jour le timer de jeu
    auto& game_timers = registry.getEntities<GameTimerComponent>();
    for (auto timer_entity : game_timers) {
        auto& timer = registry.getComponent<GameTimerComponent>(timer_entity);
        timer.elapsed_time += context.dt;
    }

    // Nettoyage des entités hors écran
    auto& entities = registry.getEntities<transform_component_s>();
    std::vector<Entity> to_cleanup;
    for (auto entity : entities) {
        if (!registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = registry.getConstComponent<TagComponent>(entity);
        bool is_enemy = false;
        bool is_boss = false;
        for (const auto& tag : tags.tags) {
            if (tag == "AI" || tag == "ENEMY_PROJECTILE" || tag == "OBSTACLE") {
                is_enemy = true;
            }
            if (tag == "BOSS") {
                is_boss = true;
            }
        }
        if (!is_enemy || is_boss)
            continue;  // Ne pas détruire le boss

        auto& transform = registry.getConstComponent<transform_component_s>(entity);
        // Détruire les entités hors écran (à gauche et à droite)
        if (transform.x < -300.0f || transform.x > windowWidth + 300.0f) {
            to_cleanup.push_back(entity);
        }
    }

    // Effectuer le nettoyage
    for (auto entity : to_cleanup) {
        registry.destroyEntity(entity);
    }

    for (auto spawner : spawners) {
        auto& spawn_comp = registry.getComponent<EnemySpawnComponent>(spawner);

        if (spawn_comp.random_seed == 0) {
            spawn_comp.random_seed = static_cast<unsigned int>(std::time(nullptr));
            spawn_comp.random_state = spawn_comp.random_seed;
        }

        spawn_comp.total_time += context.dt;

        handleObstacles(registry, context, spawn_comp, windowWidth, windowHeight);

        if (handleBossSpawn(registry, context, spawn_comp))
            continue;

        if (!spawn_comp.is_active)
            continue;

        // Handle scripted spawns (authentic R-Type level)
        if (spawn_comp.use_scripted_spawns && _scripted_spawns_loaded) {
            handleScriptedSpawns(registry, context, spawn_comp);
        } else {
            // Fallback to random wave spawns
            spawn_comp.spawn_timer += context.dt;
            if (spawn_comp.spawn_timer >= _game_config.wave_interval.value()) {
                spawn_comp.spawn_timer = 0.0f;
                spawnWave(registry, context, spawn_comp, windowWidth, windowHeight);
            }
        }
    }
}

void EnemySpawnSystem::handleObstacles(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp,
                                       float windowWidth, float windowHeight) {
    float obstacle_start = _game_config.obstacle_start_time.value();
    float obstacle_end = _game_config.obstacle_end_time.value();
    float obstacle_interval = _game_config.obstacle_interval.value_or(3.0f);

    if (spawn_comp.total_time >= obstacle_start && spawn_comp.total_time < obstacle_end && !spawn_comp.boss_spawned) {
        int obstacle_count = static_cast<int>((spawn_comp.total_time - obstacle_start) / obstacle_interval);

        if (spawn_comp.wave_count < obstacle_count + 100) {
            float y_pos = getRandomFloat(spawn_comp, 100.0f, windowHeight - 150.0f);
            spawnObstacle(registry, context, windowWidth + 100.0f, y_pos);
            spawn_comp.wave_count = obstacle_count + 100;
        }
    }
}

bool EnemySpawnSystem::handleBossSpawn(Registry& registry, system_context context, EnemySpawnComponent& spawn_comp) {
    // Vérifier si on a dépassé le temps de spawn du boss
    if (spawn_comp.total_time >= _game_config.boss_spawn_time.value() && !spawn_comp.boss_spawned) {
        // Arrêter immédiatement le spawn de nouvelles vagues
        spawn_comp.is_active = false;

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

        // Attendre que tous les ennemis soient éliminés
        if (enemy_count == 0) {
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

void EnemySpawnSystem::loadScriptedSpawns(const std::string& filepath) {
    if (_scripted_spawns_loaded)
        return;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[EnemySpawnSystem] Warning: Could not load scripted spawns from " << filepath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#')
            continue;

        // Parse spawn line: spawn=time,type,x,y,count,spacing,formation,custom_speed,custom_hp
        if (line.substr(0, 6) != "spawn=")
            continue;

        std::string data = line.substr(6);
        std::stringstream ss(data);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 6) {
            std::cerr << "[EnemySpawnSystem] Warning: Invalid spawn line: " << line << std::endl;
            continue;
        }

        SpawnEvent event;
        event.trigger_time = std::stof(tokens[0]);
        event.enemy_type = tokens[1];
        event.x_position = std::stof(tokens[2]);
        event.y_position = std::stof(tokens[3]);
        event.count = std::stoi(tokens[4]);
        event.spacing = std::stof(tokens[5]);
        event.formation = tokens.size() > 6 ? tokens[6] : "SINGLE";
        event.custom_speed = tokens.size() > 7 ? std::stof(tokens[7]) : 0.0f;
        event.custom_hp = tokens.size() > 8 ? std::stoi(tokens[8]) : 0;
        event.executed = false;

        _scripted_spawns.push_back(event);
    }

    // Sort by trigger time
    std::sort(_scripted_spawns.begin(), _scripted_spawns.end(),
              [](const SpawnEvent& a, const SpawnEvent& b) { return a.trigger_time < b.trigger_time; });

    _scripted_spawns_loaded = true;
    std::cout << "[EnemySpawnSystem] Loaded " << _scripted_spawns.size() << " scripted spawn events from " << filepath
              << std::endl;
}

void EnemySpawnSystem::handleScriptedSpawns(Registry& registry, system_context context,
                                            EnemySpawnComponent& spawn_comp) {
    float current_time = spawn_comp.total_time;

    for (auto& event : _scripted_spawns) {
        if (event.executed)
            continue;

        if (current_time >= event.trigger_time) {
            executeSpawnEvent(registry, context, event);
            event.executed = true;
        }
    }
}

void EnemySpawnSystem::executeSpawnEvent(Registry& registry, system_context context, SpawnEvent& event) {
    for (int i = 0; i < event.count; i++) {
        float x = event.x_position;
        float y = event.y_position;

        // Apply formation offsets
        if (event.formation == "LINE_HORIZONTAL") {
            x += i * event.spacing;
        } else if (event.formation == "LINE_VERTICAL") {
            y += i * event.spacing;
        } else if (event.formation == "V_FORMATION") {
            x += i * event.spacing;
            y += (i % 2 == 0 ? 1 : -1) * (i / 2 + 1) * (event.spacing / 2);
        } else if (event.formation == "SNAKE") {
            x += i * event.spacing;
            y += (i % 2 == 0 ? 50 : -50);
        }
        // SINGLE formation: no offset

        // Spawn with custom config if provided
        if (event.custom_speed > 0 || event.custom_hp > 0) {
            spawnEnemyWithConfig(registry, context, x, y, event.enemy_type, event.custom_speed, event.custom_hp);
        } else {
            spawnEnemy(registry, context, x, y, event.enemy_type);
        }
    }

    std::cout << "[EnemySpawnSystem] Spawned " << event.count << " " << event.enemy_type << " at time "
              << event.trigger_time << "s" << std::endl;
}

void EnemySpawnSystem::spawnEnemyWithConfig(Registry& registry, system_context context, float x, float y,
                                            const std::string& enemy_type, float custom_speed, int custom_hp) {
    auto it = _enemy_configs.find(enemy_type);
    if (it == _enemy_configs.end())
        return;

    // Create a copy of the config with custom values
    EntityConfig config = it->second;
    if (custom_speed > 0)
        config.speed = custom_speed;
    if (custom_hp > 0)
        config.hp = custom_hp;

    auto spawner_it = _spawners.find(enemy_type);
    if (spawner_it != _spawners.end()) {
        spawner_it->second->spawn(registry, context, x, y, config);
    }
}
