#include <gtest/gtest.h>
#include "../src/ecs/common/Registry/registry.hpp"
#include "../src/ecs/common/shoot_feature/shooter.hpp"
#include "../src/ecs/common/team_component/team_component.hpp"
#include "../src/ecs/common/Components/Components.hpp"

class ShooterTest : public ::testing::Test {
protected:
    Registry registry;
    ShooterSystem shooterSys;
    
    // Mock context
    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {0.0f, texture_manager, window};

    Entity player;

    void SetUp() override {
        player = registry.createEntity();
        // Setup minimum du joueur pour tirer
        registry.addComponent(player, transform_component_s{100.0f, 100.0f});
        registry.addComponent(player, TeamComponent{TeamComponent::ALLY});
        // FireRate = 1.0 seconde, dernier tir = 0.0
        registry.addComponent(player, ShooterComponent{ShooterComponent::NORMAL, 1.0, 0.0});
    }
};

TEST_F(ShooterTest, NoShootIfCooldownNotReady) {
    // ARRANGE
    context.dt = 0.5f; // Temps actuel = 0.5s ( < 1.0s fire rate)

    // ACT
    shooterSys.update(registry, context);

    // ASSERT
    // Aucune entité Projectile ne doit avoir été créée
    // (On suppose que le registry était vide de projectiles avant)
    bool hasProjectile = false;
    try {
        auto& projectiles = registry.getEntities<ProjectileComponent>();
        if (!projectiles.empty()) hasProjectile = true;
    } catch (...) {}

    EXPECT_FALSE(hasProjectile) << "Le joueur ne devrait pas tirer avant la fin du cooldown";
}

TEST_F(ShooterTest, ShootIfCooldownReady) {
    // ARRANGE
    context.dt = 1.5f; // Temps actuel = 1.5s ( > 1.0s fire rate)

    // ACT
    shooterSys.update(registry, context);

    // ASSERT
    // 1. Un projectile doit exister
    auto& projectiles = registry.getEntities<ProjectileComponent>();
    ASSERT_FALSE(projectiles.empty());
    
    Entity bulletID = projectiles.back();

    // 2. Vérification des propriétés du projectile
    auto& projTransform = registry.getComponent<transform_component_s>(bulletID);
    auto& projVel = registry.getComponent<VelocityComponent>(bulletID);
    auto& projTeam = registry.getComponent<TeamComponent>(bulletID);

    // Position : Doit être décalée par rapport au joueur (x + 1.0)
    EXPECT_FLOAT_EQ(projTransform.x, 101.0f); 
    EXPECT_FLOAT_EQ(projTransform.y, 100.0f);

    // Vitesse : Type NORMAL = 5 (selon ton switch)
    EXPECT_FLOAT_EQ(projVel.vx, 5.0f); 

    // Equipe : Doit être la même que le tireur
    EXPECT_EQ(projTeam.team, TeamComponent::ALLY);
}

TEST_F(ShooterTest, CooldownIsResetAfterShooting) {
    // ARRANGE
    context.dt = 1.5f;
    shooterSys.update(registry, context); // Tir effectué

    // ACT
    // On avance un tout petit peu le temps (1.6s). 
    // 1.6 - 1.5 (last_shot) = 0.1 < 1.0 (FireRate). Ne doit pas tirer.
    context.dt = 1.6f; 
    
    // On vide les projectiles pour vérifier si un nouveau apparaît
    // (Simulation simple ou check count)
    size_t countBefore = registry.getEntities<ProjectileComponent>().size();
    
    shooterSys.update(registry, context);
    
    size_t countAfter = registry.getEntities<ProjectileComponent>().size();

    // ASSERT
    EXPECT_EQ(countBefore, countAfter) << "Le cooldown n'a pas été reset, il a tiré deux fois de suite !";
}

TEST_F(ShooterTest, MultipleShotsOverTime) {
    // ARRANGE
    context.dt = 0.0f;
    float totalTime = 5.0f; // Simuler 5 secondes
    float timeStep = 0.5f;  // Mise à jour toutes les 0.5 secondes

    size_t expectedShots = 0;

    // ACT
    for (float t = 0.0f; t < totalTime; t += timeStep) {
        context.dt = t;
        shooterSys.update(registry, context);

        // Calculer le nombre de tirs attendus
        if (t >= (expectedShots + 1) * 1.0f) { // FireRate = 1.0s
            expectedShots++;
        }
    }

    // ASSERT
    auto& projectiles = registry.getEntities<ProjectileComponent>();
    EXPECT_EQ(projectiles.size(), expectedShots) << "Le nombre de projectiles tirés ne correspond pas au nombre attendu.";
}
