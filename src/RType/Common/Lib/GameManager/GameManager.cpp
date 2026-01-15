#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"

GameManager::GameManager() {
    try {
        _master_config = ConfigLoader::loadMasterConfig("src/RType/Common/content/config/master.cfg");
        _player_config = ConfigLoader::loadEntityConfig(_master_config.player_config,
                                                        ConfigLoader::getRequiredPlayerFields());
        _game_config = ConfigLoader::loadGameConfig(_master_config.game_config,
                                                    ConfigLoader::getRequiredGameFields());
        
        if (!_master_config.levels.empty()) {
            _current_level_scene = _master_config.levels[0];
        } else {
            _current_level_scene = "src/RType/Common/content/config/level1.scene";
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading master config: " << e.what() << std::endl;
        _current_level_scene = "src/RType/Common/content/config/level1.scene";
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

    LevelConfig level_config;
    try {
        level_config = SceneLoader::loadFromFile(_current_level_scene);
        
        if (!level_config.game_config.empty()) {
            _game_config = ConfigLoader::loadGameConfig(level_config.game_config,
                                                        ConfigLoader::getRequiredGameFields());
        }
    } catch (const std::exception& e) {
        std::cerr << "[GameManager] Error loading level config: " << e.what() << std::endl;
    }

    initBackground(env, level_config);
    initBounds(env);
    initPlayer(env);
    initSpawner(env, level_config);
    initScene(env, level_config);
    initUI(env);

    if (!env.isServer()) {
        auto& ecs = env.getECS();
        Entity musicEntity = ecs.registry.createEntity();
        AudioSourceComponent music;
        music.sound_name = level_config.music_track.empty() ? "theme" : level_config.music_track;
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
