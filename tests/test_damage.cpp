#include <gtest/gtest.h>

#include "box_collision/box_collision.hpp"
#include "damage_feature/damage.hpp"
#include "ecs/Registry/registry.hpp"
#include "health_feature/health.hpp"
#include "shoot_feature/shooter.hpp"  // Pour ProjectileComponent
#include "team_component/team_component.hpp"

class DamageTest : public ::testing::Test {
   protected:
    Registry registry;
    Damage damageSys;

    Entity attacker;
    Entity victim;

    void SetUp() override {
        attacker = registry.createEntity();
        victim = registry.createEntity();

        registry.addComponent(attacker, BoxCollisionComponent{});
        registry.addComponent(attacker, DamageOnCollision{10});
    }
};

TEST_F(DamageTest, EnemyHitsPlayer) {
    registry.addComponent(victim, HealthComponent{100, 100});
    registry.addComponent(victim, TeamComponent{TeamComponent::ALLY});

    registry.addComponent(attacker, TeamComponent{TeamComponent::ENEMY});
    registry.addComponent(attacker, DamageOnCollision{20});

    auto& box = registry.getComponent<BoxCollisionComponent>(attacker);
    box.collision.tags.push_back(victim);

    damageSys.update(registry, 0.0);

    auto& hp = registry.getComponent<HealthComponent>(victim);
    EXPECT_EQ(hp.current_hp, 80) << "100 HP - 20 Damage = 80 HP";
}

TEST_F(DamageTest, FriendlyFireIsIgnored) {
    registry.addComponent(victim, HealthComponent{100, 100});
    registry.addComponent(victim, TeamComponent{TeamComponent::ALLY});

    registry.addComponent(attacker, TeamComponent{TeamComponent::ALLY});
    registry.addComponent(attacker, DamageOnCollision{1000});

    auto& box = registry.getComponent<BoxCollisionComponent>(attacker);
    box.collision.tags.push_back(victim);

    damageSys.update(registry, 0.0);

    auto& hp = registry.getComponent<HealthComponent>(victim);
    EXPECT_EQ(hp.current_hp, 100) << "No ally-on-ally damage should occur";
}

TEST_F(DamageTest, ProjectileIsDestroyedAfterHit) {
    registry.addComponent(victim, HealthComponent{100, 100});
    registry.addComponent(victim, TeamComponent{TeamComponent::ALLY});

    registry.addComponent(attacker, TeamComponent{TeamComponent::ENEMY});
    registry.addComponent(attacker, ProjectileComponent{});  // C'est un missile

    auto& box = registry.getComponent<BoxCollisionComponent>(attacker);
    box.collision.tags.push_back(victim);

    damageSys.update(registry, 0.0);

    EXPECT_FALSE(registry.hasComponent<ProjectileComponent>(attacker))
        << "Projectile should be destroyed after hitting an entity";
}

TEST_F(DamageTest, DamageDoesNotGoBelowZero) {
    registry.addComponent(victim, HealthComponent{50, 10});
    registry.addComponent(victim, TeamComponent{TeamComponent::ALLY});

    registry.addComponent(attacker, TeamComponent{TeamComponent::ENEMY});
    registry.addComponent(attacker, DamageOnCollision{20});

    auto& box = registry.getComponent<BoxCollisionComponent>(attacker);
    box.collision.tags.push_back(victim);

    damageSys.update(registry, 0.0);

    auto& hp = registry.getComponent<HealthComponent>(victim);
    EXPECT_EQ(hp.current_hp, 0) << "Health should not go below zero";
}

TEST_F(DamageTest, MultipleHitsAccumulateDamage) {
    registry.addComponent(victim, HealthComponent{200, 200});
    registry.addComponent(victim, TeamComponent{TeamComponent::ALLY});

    registry.addComponent(attacker, TeamComponent{TeamComponent::ENEMY});
    registry.addComponent(attacker, DamageOnCollision{30});

    auto& box = registry.getComponent<BoxCollisionComponent>(attacker);
    box.collision.tags.push_back(victim);

    for (int i = 0; i < 3; ++i) {
        damageSys.update(registry, 0.0);
    }

    auto& hp = registry.getComponent<HealthComponent>(victim);
    EXPECT_EQ(hp.current_hp, 110) << "200 HP - (3 * 30 Damage) = 110 HP";
}
