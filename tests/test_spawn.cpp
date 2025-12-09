#include <gtest/gtest.h>

#include "../src/ecs/common/Components/Components.hpp"
#include "../src/ecs/common/Registry/registry.hpp"
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
    // ARRANGE
    registry.addComponent(spawner, SpawnComponent{5.0,  // 5 secondes avant spawn
                                                  true,
                                                  {TypeEntityComponent::ENEMY_BASIC},
                                                  transform_component_s{100, 100},
                                                  Velocity2D{0, 0}});

    // ACT
    context.dt = 1.0f;
    spawnSys.update(registry, context);

    // ASSERT
    auto& spawn = registry.getComponent<SpawnComponent>(spawner);
    EXPECT_FLOAT_EQ(spawn.time_until_spawn, 4.0);

    // CORRECTION ICI : On vérifie si la liste est vide, pas si ça crash
    bool hasEnemies = false;
    try {
        auto& enemies = registry.getEntities<TeamComponent>();
        if (!enemies.empty())
            hasEnemies = true;
    } catch (...) {
        // Si ça throw, c'est que le pool n'existe pas, donc pas d'ennemis -> OK
        hasEnemies = false;
    }

    EXPECT_FALSE(hasEnemies) << "Rien ne doit spawner tant que le timer n'est pas fini";
}

TEST_F(SpawnTest, TimerZero_SpawnsEntityAndDestroysSpawner) {
    // ARRANGE
    registry.addComponent(spawner,
                          SpawnComponent{0.5,  // 0.5 secondes avant spawn
                                         true,
                                         {TypeEntityComponent::ENEMY_BASIC},  // On veut spawner un ENNEMI BASIC
                                         transform_component_s{500, 500},
                                         Velocity2D{-10, 0}});

    // ACT
    context.dt = 1.0f;  // On avance de 1s (donc > 0.5s)
    spawnSys.update(registry, context);

    // ASSERT
    // 1. Le Spawner doit être détruit
    bool spawnerExists = true;
    try {
        registry.getComponent<SpawnComponent>(spawner);
    } catch (...) {
        spawnerExists = false;
    }
    EXPECT_FALSE(spawnerExists) << "L'entité Spawner doit être détruite après usage";

    // 2. Une nouvelle entité doit exister
    auto& enemies = registry.getEntities<TeamComponent>();  // Les ennemis ont une Team
    ASSERT_EQ(enemies.size(), 1);

    Entity newEnemy = enemies[0];

    // 3. Vérifier que la Factory a bien marché (Vérif des composants spécifiques à ENEMY_BASIC)
    EXPECT_EQ(registry.getComponent<TeamComponent>(newEnemy).team, TeamComponent::ENEMY);
    EXPECT_EQ(registry.getComponent<HealthComponent>(newEnemy).max_hp, 20);  // Basic = 20HP

    // Vérifier que le transform du spawner a été copié
    auto& pos = registry.getComponent<transform_component_s>(newEnemy);
    EXPECT_FLOAT_EQ(pos.x, 500);
}