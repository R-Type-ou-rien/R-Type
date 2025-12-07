#include <gtest/gtest.h>

#include "../src/ecs/Registry/registry.hpp"
#include "../src/health_feature/health.hpp"
#include "../src/team_component/team_component.hpp"
#include "../src/transform_component/transform.hpp"

class HealthTest : public ::testing::Test {
   protected:
    Registry registry;
    SlotMap<sf::Texture> texture_manager;
    HealthSystem healthSys;
    Entity entity;

    void SetUp() override { entity = registry.createEntity(); }
};

TEST_F(HealthTest, EntityDiesAtZeroHP) {
    system_context context = {0, texture_manager};

    registry.addComponent(entity, HealthComponent{50, 0});
    registry.addComponent(entity, TeamComponent{TeamComponent::ENEMY});

    healthSys.update(registry, context);

    EXPECT_FALSE(registry.hasComponent<HealthComponent>(entity));
}

TEST_F(HealthTest, PlayerRespawnsWithLives) {
    system_context context = {0, texture_manager};

    registry.addComponent(entity, HealthComponent{100, 0});
    registry.addComponent(entity, LifeComponent{3});
    registry.addComponent(entity, TransformComponent{999, 999});
    registry.addComponent(entity, TeamComponent{TeamComponent::ALLY});

    healthSys.update(registry, context);

    auto& hp = registry.getComponent<HealthComponent>(entity);
    auto& life = registry.getComponent<LifeComponent>(entity);
    auto& pos = registry.getComponent<TransformComponent>(entity);

    EXPECT_EQ(life.lives_remaining, 2) << "Il devrait rester 2 vies";
    EXPECT_EQ(hp.current_hp, 100) << "HP reload";
    EXPECT_EQ(pos.x, 100) << "Position X reset";

    EXPECT_TRUE(registry.hasComponent<HealthComponent>(entity));
}

TEST_F(HealthTest, PlayerDiesPermanentlyNoLives) {
    system_context context = {0, texture_manager};

    registry.addComponent(entity, HealthComponent{100, -10});
    registry.addComponent(entity, LifeComponent{0});
    registry.addComponent(entity, TeamComponent{TeamComponent::ALLY});

    healthSys.update(registry, context);

    EXPECT_FALSE(registry.hasComponent<HealthComponent>(entity)) << "Player should be dead permanently";
}
