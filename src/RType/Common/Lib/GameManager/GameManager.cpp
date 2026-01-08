#include "GameManager.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"

GameManager::GameManager() {
    _player_config = ConfigLoader::loadEntityConfig("src/RType/Common/content/config/player.cfg",
                                                    ConfigLoader::getRequiredPlayerFields());
}

void GameManager::init(Environment& env, InputManager& inputs) {
    initSystems(env);

    initBackground(env);
    initBounds(env);
    initPlayer(env);
    initSpawner(env);
    initUI(env);

    loadInputSetting(inputs);
}

void GameManager::update(Environment& env, InputManager& inputs) {
    updateUI(env);
    checkGameState(env);
}
