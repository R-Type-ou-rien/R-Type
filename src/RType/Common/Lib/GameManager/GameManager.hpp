#include <memory>
#include <vector>
#include "../Actors/Player/Player.hpp"
#include "../Actors/AI/AI.hpp"
#include "ECS.hpp"


class GameManager {
    private:
        std::unique_ptr<Player> _player;
        std::vector<std::unique_ptr<AI>> _ennemies;
    
    public:
        GameManager();
        void init(ECS& ecs);
        void update(ECS& ecs);
        void loadInputSetting(ECS& ecs);

};