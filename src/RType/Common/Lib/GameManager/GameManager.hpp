#include <memory>
#include <vector>
#include <SFML/System/Clock.hpp>
#include "../Actors/Player/Player.hpp"
#include "../Actors/AI/AI.hpp"
#include "ECS.hpp"

class GameManager {
   private:
    static constexpr float PLAYER_SPEED = 100.0f;
    static constexpr float PLAYER_START_X = 100.0f;
    static constexpr float PLAYER_START_Y = 300.0f;
    static constexpr int PLAYER_MAX_HP = 100;
    
    std::unique_ptr<Player> _player;
    std::vector<std::unique_ptr<AI>> _ennemies;
    Entity _uiEntity;

    void initSystems(ECS& ecs);
    void initBackground(ECS& ecs, ResourceManager<TextureAsset>& textures);
    void initPlayer(ECS& ecs, ResourceManager<TextureAsset>& textures);
    void initEnemies(ECS& ecs, ResourceManager<TextureAsset>& textures);
    void initSpawner(ECS& ecs);
    void initUI(ECS& ecs);
    void setupMovementControls(InputManager& inputs);
    void setupShootingControls(InputManager& inputs);
    void updateUI(ECS& ecs);

   public:
    GameManager();
    void init(ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures);
    void update(ECS& ecs, InputManager& inputs, ResourceManager<TextureAsset>& textures);
    void loadInputSetting(InputManager& inputs);
};