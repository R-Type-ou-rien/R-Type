#include <gtest/gtest.h>

#include "../src/ecs/common/Components/Components.hpp"
#include "../src/Engine/Core/ECS/Registry/registry.hpp"
#include "../src/ecs/common/health_feature/health.hpp"
#include "../src/ecs/common/spawn_feature/spawn.hpp"
#include "../src/ecs/common/team_component/team_component.hpp"

class SpawnTest : public ::testing::Test {
   protected:
    Registry registry;
    SpawnSystem spawnSys;  // Le constructeur va initialiser la Map de factories

    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {0.0f, texture_manager, window};

    Entity spawner;

    void SetUp() override { spawner = registry.createEntity(); }
};

TEST_F(SpawnTest, TimerDecreases_NoSpawnYet) {
    registry.addComponent(
        spawner, SpawnComponent{
                     5.0, true, {TypeEntityComponent::ENEMY_BASIC}, transform_component_s{100, 100}, Velocity2D{0, 0}});

    context.dt = 1.0f;
    spawnSys.update(registry, context);

    auto& spawn = registry.getComponent<SpawnComponent>(spawner);
    EXPECT_FLOAT_EQ(spawn.time_until_spawn, 4.0);

    bool hasEnemies = false;
    try {
        auto& enemies = registry.getEntities<TeamComponent>();
        if (!enemies.empty())
            hasEnemies = true;
    } catch (...) {
        hasEnemies = false;
    }

    EXPECT_FALSE(hasEnemies) << "Rien ne doit spawner tant que le timer n'est pas fini";
}

TEST_F(SpawnTest, TimerZero_SpawnsEntityAndDestroysSpawner) {
    registry.addComponent(
        spawner,
        SpawnComponent{
            0.5, true, {TypeEntityComponent::ENEMY_BASIC}, transform_component_s{500, 500}, Velocity2D{-10, 0}});

    context.dt = 1.0f;
    spawnSys.update(registry, context);

    bool spawnerExists = true;
    try {
        registry.getComponent<SpawnComponent>(spawner);
    } catch (...) {
        spawnerExists = false;
    }
    EXPECT_FALSE(spawnerExists) << "L'entité Spawner doit être détruite après usage";

    auto& enemies = registry.getEntities<TeamComponent>();  // Les ennemis ont une Team
    ASSERT_EQ(enemies.size(), 1);

    Entity newEnemy = enemies[0];

    EXPECT_EQ(registry.getComponent<TeamComponent>(newEnemy).team, TeamComponent::ENEMY);
    EXPECT_EQ(registry.getComponent<HealthComponent>(newEnemy).max_hp, 20);  // Basic = 20HP

    auto& pos = registry.getComponent<transform_component_s>(newEnemy);
    EXPECT_FLOAT_EQ(pos.x, 500);
}
