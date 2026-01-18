#include <gtest/gtest.h>

#include "../src/Engine/Core/ECS/Registry/registry.hpp"
#include "../src/ecs/common/health_feature/health.hpp"
#include "../src/ecs/common/hierarchy_feature/hierarchy.hpp"

class HierarchyTest : public ::testing::Test {
   protected:
    Registry registry;
    HierarchySystem hierarchySys;

    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {0.0f, texture_manager, window};

    Entity parent;
    Entity child;

    void SetUp() override {
        parent = registry.createEntity();
        child = registry.createEntity();

        registry.addComponent(parent, HealthComponent{100, 100});
        registry.addComponent(child, HealthComponent{100, 100});
    }
};

TEST_F(HierarchyTest, ParentDies_ChildMustDie) {
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child)}, true});

    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, false});

    hierarchySys.update(registry, context);

    bool parentExists = true;
    bool childExists = true;

    try {
        registry.getComponent<HealthComponent>(parent);
    } catch (...) {
        parentExists = false;
    }
    try {
        registry.getComponent<HealthComponent>(child);
    } catch (...) {
        childExists = false;
    }

    EXPECT_FALSE(parentExists) << "Le parent aurait dû être détruit";
    EXPECT_FALSE(childExists) << "L'enfant aurait dû être détruit par cascade";
}

TEST_F(HierarchyTest, ChildDies_ParentMustDie) {
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child)}, false});

    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, true});

    hierarchySys.update(registry, context);

    bool parentExists = true;
    try {
        registry.getComponent<HealthComponent>(parent);
    } catch (...) {
        parentExists = false;
    }

    EXPECT_FALSE(parentExists) << "Le parent aurait dû être détruit car son enfant est mort";
}

TEST_F(HierarchyTest, NoDeath_NoEntitiesDestroyed) {
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child)}, false});

    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, false});

    hierarchySys.update(registry, context);

    bool parentExists = true;
    bool childExists = true;

    try {
        registry.getComponent<HealthComponent>(parent);
    } catch (...) {
        parentExists = false;
    }
    try {
        registry.getComponent<HealthComponent>(child);
    } catch (...) {
        childExists = false;
    }

    EXPECT_TRUE(parentExists) << "Le parent ne devrait pas être détruit";
    EXPECT_TRUE(childExists) << "L'enfant ne devrait pas être détruit";
}

TEST_F(HierarchyTest, ParentAndMultipleChildren_OneChildDies_AllDestroyed) {
    Entity child2 = registry.createEntity();
    registry.addComponent(child2, HealthComponent{100, 100});

    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child), static_cast<int>(child2)}, false});

    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, true});

    registry.addComponent(child2, HierarchyComponent{static_cast<int>(parent), {}, false});

    hierarchySys.update(registry, context);

    bool parentExists = true;
    bool child1Exists = true;
    bool child2Exists = true;

    try {
        registry.getComponent<HealthComponent>(parent);
    } catch (...) {
        parentExists = false;
    }
    try {
        registry.getComponent<HealthComponent>(child);
    } catch (...) {
        child1Exists = false;
    }
    try {
        registry.getComponent<HealthComponent>(child2);
    } catch (...) {
        child2Exists = false;
    }

    EXPECT_FALSE(parentExists) << "Le parent aurait dû être détruit";
    EXPECT_FALSE(child1Exists) << "L'enfant 1 aurait dû être détruit";
    EXPECT_FALSE(child2Exists) << "L'enfant 2 aurait dû être détruit par cascade";
}

TEST_F(HierarchyTest, DeadEntityWithoutHierarchyComponent_NoCrash) {
    Entity loneEntity = registry.createEntity();
    registry.addComponent(loneEntity, HealthComponent{100, 100});

    EXPECT_NO_THROW(hierarchySys.update(registry, context));
}
