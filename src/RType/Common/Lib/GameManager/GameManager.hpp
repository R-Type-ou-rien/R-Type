#pragma once  // Use pragma once for modern C++

#include <memory>
#include <vector>
#include "ECS.hpp"
#include "GameEngineBase.hpp"
#include "GameEngineConfig.hpp"
#include "ServerGameEngine.hpp"  // Required for lobby access
#include "InputConfig.hpp"
#include "AActor.hpp"  // Needed for unique_ptr<AActor> destruction

class GameManager {
   private:
    static constexpr float PLAYER_SPEED = 100.0f;
    static constexpr float PLAYER_START_X = 100.0f;
    static constexpr float PLAYER_START_Y = 300.0f;
    static constexpr int PLAYER_MAX_HP = 100;

    Entity _uiEntity;

    uint32_t _requiredPlayers;
    bool _gameStarted = false;

    std::vector<std::unique_ptr<AActor>> _actors;

    void initSystems(GameEngine& engine);
    void initBackground(GameEngine& engine);
    void initPlayers(GameEngine& engine);
    void checkNewPlayers(GameEngine& engine);
    void initEnemies(GameEngine& engine);
    void initSpawner(GameEngine& engine);
    void initUI(GameEngine& engine);
    void updateUI(GameEngine& engine);

   public:
    explicit GameManager(uint32_t requiredPlayers = 1);

    void init(GameEngine& engine, InputManager& inputs);
    void update(GameEngine& engine, InputManager& inputs);

    bool isGameReady(GameEngine& engine);
    void loadInputSetting(InputManager& inputs);
};