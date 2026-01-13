#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"

GameManager::GameManager() {
    _player_config = ConfigLoader::loadEntityConfig("src/RType/Common/content/config/player.cfg",
                                                    ConfigLoader::getRequiredPlayerFields());
    _current_level_scene = "src/RType/Common/content/config/level1.scene";
    
    // Réinitialiser les flags de jeu
    _gameOver = false;
    _victory = false;
    _leaderboardDisplayed = false;
}

void GameManager::init(Environment& env, InputManager& inputs) {
    initSystems(env);

    env.loadGameResources("src/RType/Common/content/config/r-type.json");

    LevelConfig level_config;
    try {
        level_config = SceneLoader::loadFromFile(_current_level_scene);
    } catch (...) {
        // Fallback or handle error
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

        // Input bindings only needed on client - server players have bindings set in Player constructor
        loadInputSetting(inputs);
    }
}

void GameManager::update(Environment& env, InputManager& inputs) {
    updateUI(env);
    checkGameState(env);
}
