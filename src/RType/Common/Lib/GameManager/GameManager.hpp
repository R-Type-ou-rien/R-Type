#pragma once

#include <memory>
#include <vector>
#include <string>
#include <SFML/System/Clock.hpp>
#include "../../Entities/Player/Player.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "InputConfig.hpp"
#include "InputState.hpp"
#include "src/RType/Common/Components/config.hpp"
#include "src/RType/Common/Components/game_timer.hpp"
#include "src/Engine/Core/Scene/SceneManager.hpp"

#define SPRITE_SIZE 64

struct InputSnapshot;

class GameManager {
   private:
    std::unique_ptr<Player> _player;
    std::unique_ptr<SceneManager> _scene_manager;
    Entity _timerEntity;
    Entity _bossHPEntity;  // Nouvelle entité pour la vie du boss
    Entity _gameStateEntity;
    Entity _boundsEntity;
    Entity _scoreTrackerEntity;
    Entity _statusDisplayEntity;
    Entity _chargeBarEntity;
    Entity _livesEntity;
    Entity _scoreDisplayEntity;
    bool _gameOver = false;
    bool _victory = false;

    EntityConfig _player_config;
    std::string _current_level_scene;

    void initSystems(Environment& env);
    void initBackground(Environment& env, const LevelConfig& config);
    void initPlayer(Environment& env);
    void initSpawner(Environment& env, const LevelConfig& config);
    void initScene(Environment& env, const LevelConfig& config);
    void initUI(Environment& env);
    void initBounds(Environment& env);
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void setupPodControls(InputManager& inputs);
    void updateUI(Environment& env);
    void checkGameState(Environment& env);
    void displayGameOver(Environment& env, bool victory);

   public:
    GameManager();
    void init(Environment& env, InputManager& inputs);
    void update(Environment& env, InputManager& inputs);
    void loadInputSetting(InputManager& inputs);
    void predictionLogic(Entity e, Registry& r, const InputSnapshot& inputs, float dt);
};
