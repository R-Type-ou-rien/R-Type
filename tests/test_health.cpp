#include <gtest/gtest.h>

#include "../src/Engine/Core/ECS/Registry/registry.hpp"
#include "../src/ecs/common/health_feature/health.hpp"
#include "../src/ecs/common/team_component/team_component.hpp"

class HealthTest : public ::testing::Test {
   protected:
    Registry registry;
    HealthSystem healthSys;
    Entity entity;
    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;

    void SetUp() override { entity = registry.createEntity(); }
};

TEST_F(HealthTest, EntityDiesAtZeroHP) {
    system_context context = {0, texture_manager, window};

    registry.addComponent(entity, HealthComponent{50, 0});
    registry.addComponent(entity, TeamComponent{TeamComponent::ENEMY});

    healthSys.update(registry, context);

    EXPECT_FALSE(registry.hasComponent<HealthComponent>(entity));
}

TEST_F(HealthTest, EntitySurvivesWithPositiveHP) {
    system_context context = {0, texture_manager, window};

    registry.addComponent(entity, HealthComponent{50, 10});
    registry.addComponent(entity, TeamComponent{TeamComponent::ENEMY});

    healthSys.update(registry, context);

    EXPECT_TRUE(registry.hasComponent<HealthComponent>(entity));
    auto& health = registry.getComponent<HealthComponent>(entity);
    EXPECT_EQ(health.current_hp, 10);
}
