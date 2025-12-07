#include <gtest/gtest.h>
#include <algorithm>

#include "../src/ecs/Components/Components.hpp"
#include "../src/box_collision/box_collision.hpp"
#include "../src/ecs/Registry/registry.hpp"
#include "../src/transform_component/transform.hpp"

class CollisionTest : public ::testing::Test {
   protected:
    Registry registry;
    BoxCollision boxSystem;
    SlotMap<sf::Texture> texture_manager;

    Entity entityA;
    Entity entityB;

    void SetUp() override {
        entityA = registry.createEntity();
        entityB = registry.createEntity();

        registry.addComponent(entityA, BoxCollisionComponent{});
        registry.addComponent(entityB, BoxCollisionComponent{});

        registry.addComponent(entityA, sprite2D_component_s{{}, sf::IntRect({0, 0,}, {50, 50})});
        registry.addComponent(entityB, sprite2D_component_s{{}, sf::IntRect({0, 0,}, {50, 50})});
    }
};

TEST_F(CollisionTest, NoCollisionWhenFarApart) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{1000.0f, 1000.0f});
    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    EXPECT_TRUE(boxA.collision.tags.empty());
}

TEST_F(CollisionTest, DetectsSimpleOverlap) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{10.0f, 10.0f});
    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    auto& boxB = registry.getComponent<BoxCollisionComponent>(entityB);
    ASSERT_FALSE(boxA.collision.tags.empty());
    EXPECT_EQ(boxA.collision.tags[0], entityB);
    ASSERT_FALSE(boxB.collision.tags.empty());
    EXPECT_EQ(boxB.collision.tags[0], entityA);
}

TEST_F(CollisionTest, ScaleIncreaseHitbox) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});  // Scale défaut 1.0
    registry.addComponent(entityB, TransformComponent{60.0f, 0.0f});
    boxSystem.update(registry, context);
    EXPECT_TRUE(registry.getComponent<BoxCollisionComponent>(entityA).collision.tags.empty());
    auto& transformA = registry.getComponent<TransformComponent>(entityA);
    transformA.scale = {2.0f, 1.0f};
    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    EXPECT_FALSE(boxA.collision.tags.empty()) << "Le scale x2 aurait dû provoquer une collision !";
}

TEST_F(CollisionTest, NoCollisionAfterMovingApart) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{10.0f, 10.0f});
    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    ASSERT_FALSE(boxA.collision.tags.empty());
    auto& transformB = registry.getComponent<TransformComponent>(entityB);
    transformB.x = 1000.0f;
    transformB.y = 1000.0f;
    boxSystem.update(registry, context);
    EXPECT_TRUE(boxA.collision.tags.empty());
}

TEST_F(CollisionTest, MultipleCollisionsDetected) {
    system_context context = {0, texture_manager};

    Entity entityC = registry.createEntity();
    registry.addComponent(entityC, BoxCollisionComponent{});
    registry.addComponent(entityC, sprite2D_component_s{{}, sf::IntRect({0, 0,}, {50, 50})});

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{10.0f, 10.0f});
    registry.addComponent(entityC, TransformComponent{20.0f, 20.0f});

    boxSystem.update(registry, context);

    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    ASSERT_EQ(boxA.collision.tags.size(), 2);
    EXPECT_NE(std::find(boxA.collision.tags.begin(), boxA.collision.tags.end(), entityB), boxA.collision.tags.end());
    EXPECT_NE(std::find(boxA.collision.tags.begin(), boxA.collision.tags.end(), entityC), boxA.collision.tags.end());
}

TEST_F(CollisionTest, NoSelfCollision) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    EXPECT_TRUE(boxA.collision.tags.empty()) << "Une entité ne devrait pas se détecter elle-même en collision !";
}

TEST_F(CollisionTest, DifferentSizesHandledCorrectly) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{60.0f, 0.0f});

    auto& spriteB = registry.getComponent<sprite2D_component_s>(entityB);
    spriteB.dimension.size = {100, 100};

    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    ASSERT_FALSE(boxA.collision.tags.empty());
    EXPECT_EQ(boxA.collision.tags[0], entityB);
}

TEST_F(CollisionTest, HighSpeedEntitiesCollide) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{200.0f, 0.0f});

    auto& transformB = registry.getComponent<TransformComponent>(entityB);
    transformB.x = 25.0f;

    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    ASSERT_FALSE(boxA.collision.tags.empty());
    EXPECT_EQ(boxA.collision.tags[0], entityB);
}

TEST_F(CollisionTest, CollisionTagsClearedEachUpdate) {
    system_context context = {0, texture_manager};

    registry.addComponent(entityA, TransformComponent{0.0f, 0.0f});
    registry.addComponent(entityB, TransformComponent{10.0f, 10.0f});
    boxSystem.update(registry, context);
    auto& boxA = registry.getComponent<BoxCollisionComponent>(entityA);
    ASSERT_FALSE(boxA.collision.tags.empty());

    auto& transformB = registry.getComponent<TransformComponent>(entityB);
    transformB.x = 1000.0f;
    transformB.y = 1000.0f;

    boxSystem.update(registry, context);
    EXPECT_TRUE(boxA.collision.tags.empty())
        << "Les tags de collision devraient être réinitialisés à chaque mise à jour !";
}