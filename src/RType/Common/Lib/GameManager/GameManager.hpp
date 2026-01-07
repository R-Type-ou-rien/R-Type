#include <memory>
#include <vector>
#include <SFML/System/Clock.hpp>
#include "../Actors/Player/Player.hpp"
#include "../Actors/AI/AI.hpp"
#include "ECS.hpp"
#include "GameEngineBase.hpp"

class GameManager {
   private:
    static constexpr float PLAYER_SPEED = 100.0f;
    static constexpr float PLAYER_START_X = 100.0f;
    static constexpr float PLAYER_START_Y = 300.0f;
    static constexpr int PLAYER_MAX_HP = 100;

    std::unique_ptr<Player> _player;
    std::vector<std::unique_ptr<AI>> _ennemies;
    Entity _uiEntity;

    void initSystems(Environment& env);
    void initBackground(Environment& env);
    void initPlayer(Environment& env);
    void initEnemies(Environment& env);
    void initSpawner(Environment& env);
    void initUI(Environment& env);
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void updateUI(Environment& env);

   public:
    GameManager();
    void init(Environment& env, InputManager& inputs);
    void update(Environment& env, InputManager& inputs);
    void loadInputSetting(InputManager& inputs);
};