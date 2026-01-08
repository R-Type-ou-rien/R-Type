#include <memory>
#include <vector>
#include <SFML/System/Clock.hpp>
#include "../../Entities/Player/Player.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "src/RType/Common/Components/config.hpp"
#include "src/RType/Common/Components/game_timer.hpp"

class GameManager {
   private:
    std::unique_ptr<Player> _player;
    Entity _uiEntity;
    Entity _scoreEntity;
    Entity _timerEntity;
    Entity _gameStateEntity;
    Entity _boundsEntity;
    Entity _scoreTrackerEntity;
    bool _gameOver = false;
    bool _victory = false;

    EntityConfig _player_config;

    void initSystems(Environment& env);
    void initBackground(Environment& env);
    void initPlayer(Environment& env);
    void initSpawner(Environment& env);
    void initUI(Environment& env);
    void initBounds(Environment& env);
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void updateUI(Environment& env);
    void checkGameState(Environment& env);
    void displayGameOver(Environment& env, bool victory);

   public:
    GameManager();
    void init(Environment& env, InputManager& inputs);
    void update(Environment& env, InputManager& inputs);
    void loadInputSetting(InputManager& inputs);
};