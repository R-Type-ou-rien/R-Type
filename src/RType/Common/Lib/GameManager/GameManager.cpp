#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"

GameManager::GameManager() {
    _player_config = ConfigLoader::loadEntityConfig("src/RType/Common/content/config/player.cfg",
                                                    ConfigLoader::getRequiredPlayerFields());
    _current_level_scene = "src/RType/Common/content/config/level1.scene";
}

void GameManager::init(Environment& env, InputManager& inputs) {
    initSystems(env);

    env.loadGameResources("src/RType/Common/content/config/r-type.json");

    initBackground(env);
    initBounds(env);
    initPlayer(env);
    initSpawner(env);
    initScene(env);
    initUI(env);

    if (!env.isServer()) {
        auto& ecs = env.getECS();
        Entity musicEntity = ecs.registry.createEntity();
        AudioSourceComponent music;
        music.sound_name = "theme";
        music.play_on_start = true;
        music.loop = true;
        music.destroy_entity_on_finish = false;
        ecs.registry.addComponent<AudioSourceComponent>(musicEntity, music);
    }

    loadInputSetting(inputs);
}

void GameManager::update(Environment& env, InputManager& inputs) {
    updateUI(env);
    checkGameState(env);
}
