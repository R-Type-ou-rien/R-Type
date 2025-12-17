#include <memory>
#include <vector>
#include "../Actors/Player/Player.hpp"
#include "../Actors/AI/AI.hpp"
#include "ECS.hpp"
#include <map>  // Added for std::map

class GameManager {
   private:
    std::unique_ptr<Player> _player;
    std::map<uint32_t, std::shared_ptr<Player>> _players;  // Added
    std::vector<std::unique_ptr<AI>> _ennemies;

    void setupPlayerInputs(ECS& ecs, Player& player);  // Added

   public:
    void onPlayerConnect(ECS& ecs, std::uint32_t id);  // Added
    GameManager();
    void init(ECS& ecs);
    void update(ECS& ecs);
    void loadInputSetting(ECS& ecs);
};