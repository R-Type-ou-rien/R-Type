#include <gtest/gtest.h>
#include "../src/Engine/Core/ECS/Registry/registry.hpp"
#include "../src/ecs/common/ScrollSystem/ScrollSystem.hpp"
#include "../src/ecs/common/Components/Components.hpp"

class ScrollTest : public ::testing::Test {
   protected:
    Registry registry;
    ScrollSystem scrollSys;

    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {1.0f, texture_manager, window};

    void SetUp() override {}
};

TEST_F(ScrollTest, ScrollsEntityCorrectly) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{0.0f, 0.0f});
    registry.addComponent(entity, Scroll{10.0f, 5.0f, false});

    scrollSys.update(registry, context);

    auto& transform = registry.getComponent<transform_component_s>(entity);
    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 5.0f);
}

TEST_F(ScrollTest, DoesNotScrollWhenPaused) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{0.0f, 0.0f});
    registry.addComponent(entity, Scroll{10.0f, 5.0f, true});

    scrollSys.update(registry, context);

    auto& transform = registry.getComponent<transform_component_s>(entity);
    EXPECT_FLOAT_EQ(transform.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
}

TEST_F(ScrollTest, ScrollsNegativeDirection) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{100.0f, 100.0f});
    registry.addComponent(entity, Scroll{-10.0f, -20.0f, false});

    scrollSys.update(registry, context);

    auto& transform = registry.getComponent<transform_component_s>(entity);
    EXPECT_FLOAT_EQ(transform.x, 90.0f);
    EXPECT_FLOAT_EQ(transform.y, 80.0f);
}

TEST_F(ScrollTest, MultipleEntitiesScrollIndependently) {
    Entity e1 = registry.createEntity();
    registry.addComponent(e1, transform_component_s{0.0f, 0.0f});
    registry.addComponent(e1, Scroll{10.0f, 0.0f, false});

    Entity e2 = registry.createEntity();
    registry.addComponent(e2, transform_component_s{0.0f, 0.0f});
    registry.addComponent(e2, Scroll{0.0f, 20.0f, false});

    Entity e3 = registry.createEntity();
    registry.addComponent(e3, transform_component_s{0.0f, 0.0f});
    registry.addComponent(e3, Scroll{50.0f, 50.0f, true});

    scrollSys.update(registry, context);

    auto& t1 = registry.getComponent<transform_component_s>(e1);
    auto& t2 = registry.getComponent<transform_component_s>(e2);
    auto& t3 = registry.getComponent<transform_component_s>(e3);

    EXPECT_FLOAT_EQ(t1.x, 10.0f);
    EXPECT_FLOAT_EQ(t1.y, 0.0f);

    EXPECT_FLOAT_EQ(t2.x, 0.0f);
    EXPECT_FLOAT_EQ(t2.y, 20.0f);

    EXPECT_FLOAT_EQ(t3.x, 0.0f);
    EXPECT_FLOAT_EQ(t3.y, 0.0f);
}
