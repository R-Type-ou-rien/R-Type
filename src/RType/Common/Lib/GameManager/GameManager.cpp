#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"
#include "src/RType/Common/Components/leaderboard_component.hpp"
#include "src/RType/Common/Systems/spawn.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

GameManager::GameManager() {
    try {
        _master_config = ConfigLoader::loadMasterConfig("src/RType/Common/content/config/master.cfg");
        _player_config =
            ConfigLoader::loadEntityConfig(_master_config.player_config, ConfigLoader::getRequiredPlayerFields());
        _game_config = ConfigLoader::loadGameConfig(_master_config.game_config, ConfigLoader::getRequiredGameFields());

        loadLevelFiles();

        if (!_level_files.empty()) {
            _current_level_scene = _level_files[0];
            _current_level_index = 0;
        } else {
            _current_level_scene = "src/RType/Common/content/config/levels/level1.scene";
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading master config: " << e.what() << std::endl;
        _current_level_scene = "src/RType/Common/content/config/levels/level1.scene";
    }

    _gameOver = false;
    _victory = false;
    _leaderboardDisplayed = false;
}

void GameManager::init(Environment& env, InputManager& inputs) {
    initSystems(env);

    if (!_master_config.resources_config.empty()) {
        env.loadGameResources(_master_config.resources_config);
    } else {
        env.loadGameResources("src/RType/Common/content/config/r-type.json");
    }

    try {
        _current_level_config = SceneLoader::loadFromFile(_current_level_scene);

        if (!_current_level_config.game_config.empty()) {
            _game_config =
                ConfigLoader::loadGameConfig(_current_level_config.game_config, ConfigLoader::getRequiredGameFields());
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading level config: " << e.what() << std::endl;
    }

    initBackground(env, _current_level_config);
    initBounds(env);
    initPlayer(env);
    initSpawner(env, _current_level_config);
    initScene(env, _current_level_config);
    initUI(env);

    if (!env.isServer()) {
        auto& ecs = env.getECS();
        Entity musicEntity = ecs.registry.createEntity();
        AudioSourceComponent music;
        music.sound_name = _current_level_config.music_track.empty() ? "theme" : _current_level_config.music_track;
        music.play_on_start = true;
        music.loop = true;
        music.destroy_entity_on_finish = false;
        ecs.registry.addComponent<AudioSourceComponent>(musicEntity, music);

        loadInputSetting(inputs);
    }
}

void GameManager::update(Environment& env, InputManager& inputs) {
    updateUI(env);
    checkGameState(env);
}

void GameManager::loadLevelFiles() {
    _level_files.clear();

    try {
        if (!_master_config.levels.empty()) {
            std::string levels_dir = _master_config.levels[0];
            if (std::filesystem::is_directory(levels_dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(levels_dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".scene") {
                        _level_files.push_back(entry.path().string());
                    }
                }

                std::sort(_level_files.begin(), _level_files.end());

                for (const auto& level : _level_files) {
                    std::cout << "  - " << level << std::endl;
                }
            } else {
                _level_files.push_back(levels_dir);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading level files: " << e.what() << std::endl;
    }
}

void GameManager::loadNextLevel(Environment& env) {
    auto& ecs = env.getECS();

    _victory = false;
    _leaderboardDisplayed = false;
    _inTransition = true;
    std::string next_level;

    if (!_current_level_config.next_level.empty()) {
        next_level = _current_level_config.next_level;
    } else {
        _current_level_index++;

        if (_current_level_index < static_cast<int>(_level_files.size())) {
            next_level = _level_files[_current_level_index];
        } else {
            _inTransition = false;
            return;
        }
    }

    if (next_level.empty()) {
        _inTransition = false;
        return;
    }

    _current_level_scene = next_level;

    auto it = std::find(_level_files.begin(), _level_files.end(), next_level);
    if (it != _level_files.end()) {
        _current_level_index = std::distance(_level_files.begin(), it);
    }

    try {
        _current_level_config = SceneLoader::loadFromFile(_current_level_scene);
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading new level config: " << e.what() << std::endl;
        return;
    }

    std::vector<Entity> leaderboard_entities;
    auto leaderboards = ecs.registry.getEntities<LeaderboardComponent>();
    for (auto entity : leaderboards) {
        leaderboard_entities.push_back(entity);
    }
    for (auto entity : leaderboard_entities) {
        std::cout << "[GameManager] Destroying leaderboard entity " << entity << std::endl;
        ecs.registry.destroyEntity(entity);
    }

    std::vector<Entity> ui_entities_to_destroy;
    auto& all_entities = ecs.registry.getEntities<TagComponent>();

    for (auto entity : all_entities) {
        if (!ecs.registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
        for (const auto& tag : tags.tags) {
            if (tag == "UI" || tag == "LEADERBOARD" || tag == "VICTORY_TIMER") {
                ui_entities_to_destroy.push_back(entity);
                break;
            }
        }
    }

    for (auto entity : ui_entities_to_destroy) {
        ecs.registry.destroyEntity(entity);
    }

    auto spawners = ecs.registry.getEntities<EnemySpawnComponent>();
    for (auto spawner : spawners) {
        ecs.registry.destroyEntity(spawner);
    }

    auto scripted_spawners = ecs.registry.getEntities<ScriptedSpawnComponent>();
    for (auto spawner : scripted_spawners) {
        ecs.registry.destroyEntity(spawner);
    }

    auto pod_spawners = ecs.registry.getEntities<PodSpawnComponent>();
    for (auto spawner : pod_spawners) {
        ecs.registry.destroyEntity(spawner);
    }
    for (auto spawner : pod_spawners) {
        ecs.registry.destroyEntity(spawner);
    }

    auto timers = ecs.registry.getEntities<GameTimerComponent>();
    for (auto timer : timers) {
        ecs.registry.destroyEntity(timer);
    }

    std::vector<Entity> entities_to_destroy;
    all_entities = ecs.registry.getEntities<TagComponent>();

    int player_count = 0;
    int enemy_count = 0;
    int projectile_count = 0;
    int other_count = 0;

    for (auto entity : all_entities) {
        if (!ecs.registry.hasComponent<TagComponent>(entity))
            continue;
        auto& tags = ecs.registry.getConstComponent<TagComponent>(entity);
        bool should_keep = false;
        bool is_player = false;
        bool is_enemy = false;
        bool is_projectile = false;

        for (const auto& tag : tags.tags) {
            if (tag == "PLAYER") {
                is_player = true;
                should_keep = true;
                break;
            }
            if (tag == "BACKGROUND" || tag == "BOUND") {
                should_keep = true;
                break;
            }
            if (tag == "ENEMY" || tag == "BOSS")
                is_enemy = true;
            if (tag == "PROJECTILE")
                is_projectile = true;
        }

        if (!should_keep) {
            entities_to_destroy.push_back(entity);
            if (is_player)
                player_count++;
            else if (is_enemy)
                enemy_count++;
            else if (is_projectile)
                projectile_count++;
            else
                other_count++;
        }
    }

    for (auto entity : entities_to_destroy) {
        ecs.registry.destroyEntity(entity);
    }

    std::vector<Entity> untagged_health_entities;
    auto health_entities = ecs.registry.getEntities<HealthComponent>();
    for (auto entity : health_entities) {
        if (ecs.registry.hasComponent<TagComponent>(entity))
            continue;

        bool is_player = false;
        if (ecs.registry.hasComponent<NetworkIdentity>(entity)) {
            auto& netId = ecs.registry.getConstComponent<NetworkIdentity>(entity);
            if (netId.ownerId >= 0) {
                is_player = true;
            }
        }

        if (!is_player) {
            untagged_health_entities.push_back(entity);
        }
    }

    std::cout << "[GameManager] Destroying " << untagged_health_entities.size()
              << " untagged health entities (potential invisible enemies)" << std::endl;
    for (auto entity : untagged_health_entities) {
        ecs.registry.destroyEntity(entity);
    }

    size_t total_cleaned = entities_to_destroy.size() + timers.size() + scripted_spawners.size() + pod_spawners.size() +
                           untagged_health_entities.size();
    std::cout << "[GameManager] Total cleaned: " << total_cleaned << " entities" << std::endl;

    std::cout << "[GameManager] All entities cleaned, loading new level scene..." << std::endl;
    initBackground(env, _current_level_config);
    initSpawner(env, _current_level_config);
    initScene(env, _current_level_config);

    _inTransition = false;

    std::cout << "[GameManager] ===== LEVEL " << (_current_level_index + 1) << "/" << _level_files.size()
              << " LOADED SUCCESSFULLY! =====" << std::endl;
}
