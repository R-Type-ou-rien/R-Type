#include <memory>
#include <vector>
#include <SFML/System/Clock.hpp>
#include "../Actors/Player/Player.hpp"
#include "../Actors/AI/AI.hpp"
#include "ECS.hpp"

class GameManager {
   private:
    std::unique_ptr<Player> _player;
    std::vector<std::unique_ptr<AI>> _ennemies;
    Entity _uiEntity;

   public:
    GameManager();
    void init(ECS& ecs, InputManager& inputs);
    void update(ECS& ecs, InputManager& inputs);
    void loadInputSetting(InputManager& inputs);
};