#include "GameManager.hpp"
#include "ClientGameEngine.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "InputState.hpp"
#include "src/Engine/Core/Scene/SceneLoader.hpp"

GameManager::GameManager() {
    _player_config = ConfigLoader::loadEntityConfig("src/RType/Common/content/config/player.cfg",
                                                    ConfigLoader::getRequiredPlayerFields());
    _current_level_scene = "src/RType/Common/content/config/level1.scene";
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

        loadInputSetting(inputs);
    }
}

void GameManager::update(Environment& env, InputManager& inputs) {
    updateUI(env);
    checkGameState(env);
}

void GameManager::predictionLogic(Entity e, Registry& r, const InputSnapshot& inputs, float dt) {
    if (!r.hasComponent<Velocity2D>(e) || !r.hasComponent<transform_component_s>(e)) 
        return;

    auto& vel = r.getComponent<Velocity2D>(e);
    auto& pos = r.getComponent<transform_component_s>(e);

    float speed = _player_config.speed.value(); 

    vel.vx = 0;
    vel.vy = 0;

    if (inputs.isPressed("move_up"))    vel.vy = -speed;
    if (inputs.isPressed("move_down"))  vel.vy = speed;
    if (inputs.isPressed("move_left"))  vel.vx = -speed;
    if (inputs.isPressed("move_right")) vel.vx = speed;

    // Application
    pos.x += vel.vx * dt;
    pos.y += vel.vy * dt;

    if (pos.x < 0) pos.x = 0;
    if (pos.y < 0) pos.y = 0;
    if (pos.x > 1800) pos.x = WINDOW_W - SPRITE_SIZE; 
    if (pos.y > 1000) pos.y = WINDOW_H - SPRITE_SIZE;
}
