#include <gtest/gtest.h>

#include "../src/ecs/Registry/registry.hpp"
#include "../src/health_feature/health.hpp"
#include "../src/team_component/team_component.hpp"
#include "../src/transform_component/transform.hpp"

class HealthTest : public ::testing::Test {
   protected:
    Registry registry;
    ResourceManager<sf::Texture> texture_manager;
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

TEST_F(HealthTest, EntitySurvivesWithPositiveHP) {
    system_context context = {0, texture_manager};

    registry.addComponent(entity, HealthComponent{50, 10});
    registry.addComponent(entity, TeamComponent{TeamComponent::ENEMY});

    healthSys.update(registry, context);

    EXPECT_TRUE(registry.hasComponent<HealthComponent>(entity));
    auto& health = registry.getComponent<HealthComponent>(entity);
    EXPECT_EQ(health.current_hp, 10);
}
