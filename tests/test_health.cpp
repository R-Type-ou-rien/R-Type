#include <gtest/gtest.h>

#include "ecs/Registry/registry.hpp"
#include "health_feature/health.hpp"
#include "team_component/team_component.hpp"
#include "transform_component/transform.hpp"

class HealthTest : public ::testing::Test {
   protected:
    Registry registry;
    HealthSystem healthSys;
    Entity entity;

    void SetUp() override { entity = registry.createEntity(); }
};

TEST_F(HealthTest, EntityDiesAtZeroHP) {
    registry.addComponent(entity, HealthComponent{50, 0});
    registry.addComponent(entity, TeamComponent{TeamComponent::ENEMY});

    healthSys.update(registry, 0.0);

    EXPECT_FALSE(registry.hasComponent<HealthComponent>(entity));
}

TEST_F(HealthTest, PlayerRespawnsWithLives) {
    registry.addComponent(entity, HealthComponent{100, 0});
    registry.addComponent(entity, LifeComponent{3});
    registry.addComponent(entity, TransformComponent{999, 999});
    registry.addComponent(entity, TeamComponent{TeamComponent::ALLY});

    healthSys.update(registry, 0.0);

    auto& hp = registry.getComponent<HealthComponent>(entity);
    auto& life = registry.getComponent<LifeComponent>(entity);
    auto& pos = registry.getComponent<TransformComponent>(entity);

    EXPECT_EQ(life.lives_remaining, 2) << "Il devrait rester 2 vies";
    EXPECT_EQ(hp.current_hp, 100) << "HP reload";
    EXPECT_EQ(pos.x, 100) << "Position X reset";

    EXPECT_TRUE(registry.hasComponent<HealthComponent>(entity));
}

TEST_F(HealthTest, PlayerDiesPermanentlyNoLives) {
    registry.addComponent(entity, HealthComponent{100, -10});
    registry.addComponent(entity, LifeComponent{0});
    registry.addComponent(entity, TeamComponent{TeamComponent::ALLY});

    healthSys.update(registry, 0.0);

    EXPECT_FALSE(registry.hasComponent<HealthComponent>(entity)) << "Player should be dead permanently";
}
