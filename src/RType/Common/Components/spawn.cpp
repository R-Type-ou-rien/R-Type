#include "spawn.hpp"

#include <cmath>
#include <iostream>

#include "Components/StandardComponents.hpp"
#include "damage.hpp"
#include "health.hpp"
#include "shooter.hpp"
#include "team_component.hpp"

void SpawnSystem::update(Registry& registry, system_context context) {
    auto& spawners = registry.getEntities<SpawnComponent>();

    for (auto spawner : spawners) {
        auto& spawn_comp = registry.getComponent<SpawnComponent>(spawner);

        if (!spawn_comp.is_active)
            continue;

        spawn_comp.spawn_timer += context.dt;

        if (spawn_comp.spawn_timer >= spawn_comp.spawn_interval) {
            spawn_comp.spawn_timer = 0.0f;
            spawn_comp.wave_count++;

            for (int i = 0; i < spawn_comp.enemies_per_wave; i++) {
                float y_position = 100.0f + (i * 150.0f);
                bool use_sine = (i % 2 == 1);
                spawnEnemy(registry, context, 800.0f, y_position, use_sine);
            }
        }
    }
}

void SpawnSystem::spawnEnemy(Registry& registry, system_context context, float x, float y, bool sine_pattern) {
    Entity enemy_id = registry.createEntity();

    registry.addComponent<transform_component_s>(enemy_id, {x, y});

    if (sine_pattern) {
        PatternComponent pattern;
        pattern.type = PatternComponent::SINUSOIDAL;
        pattern.speed = 100.0f;
        pattern.amplitude = 50.0f;
        pattern.frequency = 2.0f;
        pattern.is_active = true;
        registry.addComponent<PatternComponent>(enemy_id, pattern);
    } else {
        registry.addComponent<Velocity2D>(enemy_id, {-150.0f, 0.0f});
    }

    registry.addComponent<HealthComponent>(enemy_id, {10, 10, 0.0f, 0.5f});

    registry.addComponent<TeamComponent>(enemy_id, {TeamComponent::ENEMY});

    registry.addComponent<DamageOnCollision>(enemy_id, {10});

    ShooterComponent shooter;
    shooter.type = ShooterComponent::NORMAL;
    shooter.is_shooting = true;
    shooter.fire_rate = 1.5;
    shooter.last_shot = 0.0f;
    registry.addComponent<ShooterComponent>(enemy_id, shooter);

    handle_t<sf::Texture> handle = context.texture_manager.load_resource(
        "content/sprites/r-typesheet42.gif", sf::Texture("content/sprites/r-typesheet42.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {32, 0, 32, 16};
    sprite_info.z_index = 1;
    registry.addComponent<sprite2D_component_s>(enemy_id, sprite_info);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(enemy_id, collision);

    TagComponent tags;
    tags.tags.push_back("AI");
    registry.addComponent<TagComponent>(enemy_id, tags);
}
