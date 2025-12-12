#include <gtest/gtest.h>

#include <cmath>

#include "../src/ecs/common/Components/Components.hpp"
#include "../src/ecs/common/Registry/registry.hpp"
#include "../src/ecs/common/pattern_system/PatternSystem.hpp"

class PatternTest : public ::testing::Test {
   protected:
    Registry registry;
    PatternSystem patternSys;

    ResourceManager<sf::Texture> texture_manager;
    sf::RenderWindow window;
    system_context context = {1.0f, texture_manager, window};  // dt = 1.0f

    void SetUp() override {}
};

TEST_F(PatternTest, MovesTowardsWaypoint) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{0.0f, 0.0f});

    PatternComponent pattern;
    pattern.speed = 10.0f;
    pattern.waypoints.push_back({10.0f, 0.0f});
    pattern.is_active = true;
    registry.addComponent(entity, pattern);

    patternSys.update(registry, context);

    auto& transform = registry.getComponent<transform_component_s>(entity);
    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
}

TEST_F(PatternTest, ReachesWaypointAndSwitches) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{98.0f, 0.0f});

    PatternComponent pattern;
    pattern.speed = 10.0f;
    pattern.waypoints.push_back({100.0f, 0.0f});    // Index 0
    pattern.waypoints.push_back({100.0f, 100.0f});  // Index 1
    pattern.is_active = true;
    registry.addComponent(entity, pattern);

    patternSys.update(registry, context);

    auto& path = registry.getComponent<PatternComponent>(entity);
    EXPECT_EQ(path.current_index, 1);
}

TEST_F(PatternTest, LoopsPattern) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{98.0f, 0.0f});

    PatternComponent pattern;
    pattern.speed = 10.0f;
    pattern.loop = true;
    pattern.waypoints.push_back({100.0f, 0.0f});  // Index 0
    pattern.is_active = true;
    registry.addComponent(entity, pattern);

    patternSys.update(registry, context);

    auto& path = registry.getComponent<PatternComponent>(entity);
    EXPECT_EQ(path.current_index, 0);
}

TEST_F(PatternTest, StopsAtEnd) {
    Entity entity = registry.createEntity();
    registry.addComponent(entity, transform_component_s{98.0f, 0.0f});

    PatternComponent pattern;
    pattern.speed = 10.0f;
    pattern.loop = false;
    pattern.waypoints.push_back({100.0f, 0.0f});  // Index 0
    pattern.is_active = true;
    registry.addComponent(entity, pattern);

    patternSys.update(registry, context);

    auto& path = registry.getComponent<PatternComponent>(entity);
    EXPECT_FALSE(path.is_active);
}
