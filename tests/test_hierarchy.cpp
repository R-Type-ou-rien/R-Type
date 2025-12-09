#include <gtest/gtest.h>
#include "../src/ecs/common/Registry/registry.hpp"
#include "../src/ecs/common/hierarchy_feature/hierarchy.hpp"
#include "../src/ecs/common/health_feature/health.hpp"

class HierarchyTest : public ::testing::Test {
protected:
    Registry registry;
    HierarchySystem hierarchySys;
    
    // Mocks
    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {0.0f, texture_manager, window};

    Entity parent;
    Entity child;

    void SetUp() override {
        parent = registry.createEntity();
        child = registry.createEntity();

        // Ajout de composants basiques pour qu'ils existent "vraiment"
        registry.addComponent(parent, HealthComponent{100, 100});
        registry.addComponent(child, HealthComponent{100, 100});
    }
};

TEST_F(HierarchyTest, ParentDies_ChildMustDie) {
    // ARRANGE
    // Configuration Parent
    // parent_id = -1, children = {child}
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child)}, true}); // is_dead = TRUE (Le parent meurt)
    
    // Configuration Enfant
    // parent_id = parent, children = {}
    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, false}); // is_dead = FALSE (L'enfant va bien)

    // ACT
    hierarchySys.update(registry, context);

    // ASSERT
    // Les deux entités doivent avoir été détruites (retirées du registry)
    // On vérifie si elles ont encore le composant Health (car destroyEntity supprime tout)
    bool parentExists = true;
    bool childExists = true;
    
    try { registry.getComponent<HealthComponent>(parent); } catch(...) { parentExists = false; }
    try { registry.getComponent<HealthComponent>(child); } catch(...) { childExists = false; }

    EXPECT_FALSE(parentExists) << "Le parent aurait dû être détruit";
    EXPECT_FALSE(childExists) << "L'enfant aurait dû être détruit par cascade";
}

TEST_F(HierarchyTest, ChildDies_ParentMustDie) {
    // ARRANGE
    // Parent vivant
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child)}, false}); 
    
    // Enfant mort (is_dead = true)
    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, true});

    // ACT
    hierarchySys.update(registry, context);

    // ASSERT
    bool parentExists = true;
    try { registry.getComponent<HealthComponent>(parent); } catch(...) { parentExists = false; }

    EXPECT_FALSE(parentExists) << "Le parent aurait dû être détruit car son enfant est mort";
}

TEST_F(HierarchyTest, NoDeath_NoEntitiesDestroyed) {
    // ARRANGE
    // Parent vivant
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child)}, false}); 
    
    // Enfant vivant
    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, false});

    // ACT
    hierarchySys.update(registry, context);

    // ASSERT
    bool parentExists = true;
    bool childExists = true;
    
    try { registry.getComponent<HealthComponent>(parent); } catch(...) { parentExists = false; }
    try { registry.getComponent<HealthComponent>(child); } catch(...) { childExists = false; }

    EXPECT_TRUE(parentExists) << "Le parent ne devrait pas être détruit";
    EXPECT_TRUE(childExists) << "L'enfant ne devrait pas être détruit";
}

TEST_F(HierarchyTest, ParentAndMultipleChildren_OneChildDies_AllDestroyed) {
    // ARRANGE
    Entity child2 = registry.createEntity();
    registry.addComponent(child2, HealthComponent{100, 100});

    // Parent
    registry.addComponent(parent, HierarchyComponent{-1, {static_cast<int>(child), static_cast<int>(child2)}, false}); 
    
    // Enfant 1 mort
    registry.addComponent(child, HierarchyComponent{static_cast<int>(parent), {}, true});

    // Enfant 2 vivant
    registry.addComponent(child2, HierarchyComponent{static_cast<int>(parent), {}, false});

    // ACT
    hierarchySys.update(registry, context);

    // ASSERT
    bool parentExists = true;
    bool child1Exists = true;
    bool child2Exists = true;
    
    try { registry.getComponent<HealthComponent>(parent); } catch(...) { parentExists = false; }
    try { registry.getComponent<HealthComponent>(child); } catch(...) { child1Exists = false; }
    try { registry.getComponent<HealthComponent>(child2); } catch(...) { child2Exists = false; }

    EXPECT_FALSE(parentExists) << "Le parent aurait dû être détruit";
    EXPECT_FALSE(child1Exists) << "L'enfant 1 aurait dû être détruit";
    EXPECT_FALSE(child2Exists) << "L'enfant 2 aurait dû être détruit par cascade";
}

TEST_F(HierarchyTest, DeadEntityWithoutHierarchyComponent_NoCrash) {
    // ARRANGE
    Entity loneEntity = registry.createEntity();
    registry.addComponent(loneEntity, HealthComponent{100, 100});
    // Pas de HierarchyComponent ajouté

    // ACT & ASSERT
    // On s'assure que l'appel ne cause pas de crash même si l'entité n'a pas de HierarchyComponent
    EXPECT_NO_THROW(hierarchySys.update(registry, context));
}
