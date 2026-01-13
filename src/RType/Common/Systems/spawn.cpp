#include "spawn.hpp"
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "Components/StandardComponents.hpp"
#include "../Entities/Mobs/all_mobs.hpp"
#include "../Components/game_timer.hpp"
#include "../Components/scripted_spawn.hpp"

const float WORLD_WIDTH = 1920.0f;
const float WORLD_HEIGHT = 1080.0f;

void EnemySpawnSystem::loadConfigs(const std::string& enemies, const std::string& boss, const std::string& game) {
    if (_configs_loaded)
        return;

    std::string enemies_path = enemies.empty() ? "src/RType/Common/content/config/enemies.cfg" : enemies;
    std::string boss_path = boss.empty() ? "src/RType/Common/content/config/boss.cfg" : boss;
    std::string game_path = game.empty() ? "src/RType/Common/content/config/game.cfg" : game;

    _enemy_configs = ConfigLoader::loadEnemiesConfig(enemies_path, ConfigLoader::getRequiredEnemyFields());
    _boss_config = ConfigLoader::loadEntityConfig(boss_path, ConfigLoader::getRequiredBossFields());
    _game_config = ConfigLoader::loadGameConfig(game_path, ConfigLoader::getRequiredGameFields());

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

void EnemySpawnSystem::loadScriptedSpawns(ScriptedSpawnComponent& scripted_spawn, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open spawn script: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("spawn=") == 0) {
            std::string content = line.substr(6);
            std::stringstream ss(content);
            std::string segment;
            std::vector<std::string> parts;

            while (std::getline(ss, segment, ',')) {
                parts.push_back(segment);
            }

            if (parts.size() >= 9) {
                SpawnEvent event;
                event.trigger_time = std::stof(parts[0]);
                event.enemy_type = parts[1];
                event.x_position = std::stof(parts[2]);
                event.y_position = std::stof(parts[3]);
                event.count = std::stoi(parts[4]);
                event.spacing = std::stof(parts[5]);
                event.formation = parts[6];
                event.custom_speed = std::stof(parts[7]);
                event.custom_hp = std::stoi(parts[8]);
                event.executed = false;

                scripted_spawn.spawn_events.push_back(event);
            }
        }
    }
    std::cout << "Loaded " << scripted_spawn.spawn_events.size() << " scripted spawn events." << std::endl;
}

void EnemySpawnSystem::handleScriptedSpawns(Registry& registry, system_context context,
                                            ScriptedSpawnComponent& scripted_spawn, float windowWidth,
                                            float windowHeight) {
    if (scripted_spawn.all_events_completed)
        return;

    scripted_spawn.level_time += context.dt;

    while (scripted_spawn.next_event_index < scripted_spawn.spawn_events.size()) {
        SpawnEvent& event = scripted_spawn.spawn_events[scripted_spawn.next_event_index];

        if (scripted_spawn.level_time >= event.trigger_time) {
            // Execute spawn event
            if (event.formation == "SINGLE") {
                spawnEnemy(registry, context, event.x_position, event.y_position, event.enemy_type);
            } else if (event.formation == "LINE_HORIZONTAL") {
                for (int i = 0; i < event.count; i++) {
                    spawnEnemy(registry, context, event.x_position + (i * event.spacing), event.y_position,
                               event.enemy_type);
                }
            } else if (event.formation == "LINE_VERTICAL") {
                for (int i = 0; i < event.count; i++) {
                    spawnEnemy(registry, context, event.x_position, event.y_position + (i * event.spacing),
                               event.enemy_type);
                }
            } else if (event.formation == "V_FORMATION") {
                for (int i = 0; i < event.count; i++) {
                    float y_offset = std::abs(i - event.count / 2) * event.spacing;
                    spawnEnemy(registry, context, event.x_position + (i * 50), event.y_position + y_offset,
                               event.enemy_type);
                }
            } else if (event.formation == "SNAKE") {
                for (int i = 0; i < event.count; i++) {
                    spawnEnemy(registry, context, event.x_position + (i * event.spacing),
                               event.y_position + std::sin(i) * 50, event.enemy_type);
                }
            }

            event.executed = true;
            scripted_spawn.next_event_index++;
        } else {
            break;  // Events are sorted by time, so we can stop checking
        }
    }

    if (scripted_spawn.next_event_index >= scripted_spawn.spawn_events.size()) {
        scripted_spawn.all_events_completed = true;
    }
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
        if (!registry.hasComponent<PendingDestruction>(entity)) {
            registry.addComponent<PendingDestruction>(entity, {});
        }
    }

    // Handle scripted spawns
    auto& scripted_spawners = registry.getEntities<ScriptedSpawnComponent>();
    for (auto spawner : scripted_spawners) {
        auto& scripted_spawn = registry.getComponent<ScriptedSpawnComponent>(spawner);

        // Load script if empty (first run)
        if (scripted_spawn.spawn_events.empty() && !scripted_spawn.all_events_completed) {
            std::string path = scripted_spawn.script_path.empty() ? "src/RType/Common/content/config/level1_spawns.cfg"
                                                                  : scripted_spawn.script_path;
            loadScriptedSpawns(scripted_spawn, path);
        }

        handleScriptedSpawns(registry, context, scripted_spawn, windowWidth, windowHeight);
    }

    for (auto spawner : spawners) {
        auto& spawn_comp = registry.getComponent<EnemySpawnComponent>(spawner);

        // Load configurations if not yet loaded
        loadConfigs(spawn_comp.enemies_config_path, spawn_comp.boss_config_path, spawn_comp.game_config_path);

        if (spawn_comp.random_seed == 0) {
            spawn_comp.random_seed = static_cast<unsigned int>(std::time(nullptr));
            spawn_comp.random_state = spawn_comp.random_seed;
        }

        spawn_comp.total_time += context.dt;

        // Obstacles avant le boss
        if (!spawn_comp.use_scripted_spawns) {
            handleObstacles(registry, context, spawn_comp, windowWidth, windowHeight);
        }

        // Gestion du boss
        if (handleBossSpawn(registry, context, spawn_comp))
            continue;

        if (!spawn_comp.is_active)
            continue;

        // Spawn des vagues normales (seulement si pas en mode scripté)
        if (!spawn_comp.use_scripted_spawns) {
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
