#include <gtest/gtest.h>

#include "../src/ecs/common/Components/Components.hpp"
#include "../src/Engine/Core/ECS/Registry/registry.hpp"
#include "../src/ecs/common/shoot_feature/shooter.hpp"
#include "../src/ecs/common/team_component/team_component.hpp"

class ShooterTest : public ::testing::Test {
   protected:
    Registry registry;
    ShooterSystem shooterSys;

    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {0.0f, texture_manager, window};

    Entity player;

    void SetUp() override {
        player = registry.createEntity();
        registry.addComponent(player, transform_component_s{100.0f, 100.0f});
        registry.addComponent(player, TeamComponent{TeamComponent::ALLY});
        registry.addComponent(player, ShooterComponent{ShooterComponent::NORMAL, 1.0, 0.0});
    }
};

TEST_F(ShooterTest, NoShootIfCooldownNotReady) {
    context.dt = 0.5f;

    shooterSys.update(registry, context);

    bool hasProjectile = false;
    try {
        auto& projectiles = registry.getEntities<ProjectileComponent>();
        if (!projectiles.empty())
            hasProjectile = true;
    } catch (...) {}

    EXPECT_FALSE(hasProjectile) << "Le joueur ne devrait pas tirer avant la fin du cooldown";
}

TEST_F(ShooterTest, ShootIfCooldownReady) {
    context.dt = 1.5f;

    shooterSys.update(registry, context);

    auto& projectiles = registry.getEntities<ProjectileComponent>();
    ASSERT_FALSE(projectiles.empty());

    Entity bulletID = projectiles.back();

    auto& projTransform = registry.getComponent<transform_component_s>(bulletID);
    auto& projVel = registry.getComponent<VelocityComponent>(bulletID);
    auto& projTeam = registry.getComponent<TeamComponent>(bulletID);

    EXPECT_FLOAT_EQ(projTransform.x, 101.0f);
    EXPECT_FLOAT_EQ(projTransform.y, 100.0f);

    EXPECT_FLOAT_EQ(projVel.vx, 5.0f);

    EXPECT_EQ(projTeam.team, TeamComponent::ALLY);
}

TEST_F(ShooterTest, CooldownIsResetAfterShooting) {
    context.dt = 1.5f;
    shooterSys.update(registry, context);

    context.dt = 1.6f;

    size_t countBefore = registry.getEntities<ProjectileComponent>().size();

    shooterSys.update(registry, context);

    size_t countAfter = registry.getEntities<ProjectileComponent>().size();

    EXPECT_EQ(countBefore, countAfter) << "Le cooldown n'a pas été reset, il a tiré deux fois de suite !";
}

TEST_F(ShooterTest, MultipleShotsOverTime) {
    context.dt = 0.0f;
    float totalTime = 5.0f;
    float timeStep = 0.5f;

    size_t expectedShots = 0;

    for (float t = 0.0f; t < totalTime; t += timeStep) {
        context.dt = t;
        shooterSys.update(registry, context);

        if (t >= (expectedShots + 1) * 1.0f) {
            expectedShots++;
        }
    }

    auto& projectiles = registry.getEntities<ProjectileComponent>();
    EXPECT_EQ(projectiles.size(), expectedShots)
        << "Le nombre de projectiles tirés ne correspond pas au nombre attendu.";
}
