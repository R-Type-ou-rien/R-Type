#include "spawn.hpp"

#include <cmath>
#include <iostream>

#include "Components/StandardComponents.hpp"
#include "damage.hpp"
#include "health.hpp"
#include "shooter.hpp"
#include "team_component.hpp"

void EnemySpawnSystem::update(Registry& registry, system_context context) {
    auto& spawners = registry.getEntities<EnemySpawnComponent>();
    float windowWidth = static_cast<float>(context.window.getSize().x);

    // Cleanup entities that are out of bounds (left side)
    auto& entities = registry.getEntities<transform_component_s>();
    for (auto entity : entities) {
        if (!registry.hasComponent<TagComponent>(entity)) continue;
        auto& tags = registry.getComponent<TagComponent>(entity);
        bool is_enemy = false;
        for (const auto& tag : tags.tags) {
            if (tag == "AI" || tag == "ENEMY_PROJECTILE") {
                is_enemy = true;
                break;
            }
        }
        if (!is_enemy) continue;

        auto& transform = registry.getComponent<transform_component_s>(entity);
        if (transform.x < -100.0f) {
            registry.destroyEntity(entity);
        }
    }

    for (auto spawner : spawners) {
        auto& spawn_comp = registry.getComponent<EnemySpawnComponent>(spawner);

        spawn_comp.total_time += context.dt;

        // Boss Logic
        if (spawn_comp.total_time >= 30.0f && !spawn_comp.boss_spawned) {
            spawn_comp.is_active = false; // Stop normal spawns
            spawn_comp.boss_spawned = true;
            spawnBoss(registry, context);
            continue;
        }

        if (!spawn_comp.is_active)
            continue;

        spawn_comp.spawn_timer += context.dt;

        if (spawn_comp.spawn_timer >= spawn_comp.spawn_interval) {
            spawn_comp.spawn_timer = 0.0f;
            spawn_comp.wave_count++;

            for (int i = 0; i < spawn_comp.enemies_per_wave; i++) {
                float y_position = 100.0f + (i * 150.0f);
                bool use_sine = (i % 2 == 1);
                spawnEnemy(registry, context, windowWidth + 50.0f, y_position, use_sine);
            }
        }
    }
}

void EnemySpawnSystem::spawnBoss(Registry& registry, system_context context) {
    Entity boss_id = registry.createEntity();
    float windowWidth = static_cast<float>(context.window.getSize().x);
    float windowHeight = static_cast<float>(context.window.getSize().y);

    registry.addComponent<transform_component_s>(boss_id, {windowWidth - 200.0f, windowHeight / 2.0f - 50.0f});
    registry.addComponent<Velocity2D>(boss_id, {0.0f, 0.0f}); // Boss just sits there for now
    registry.addComponent<HealthComponent>(boss_id, {100, 100, 0.0f, 1.0f});
    registry.addComponent<TeamComponent>(boss_id, {TeamComponent::ENEMY});
    registry.addComponent<DamageOnCollision>(boss_id, {20});

    handle_t<sf::Texture> handle = context.texture_manager.load_resource(
        "content/sprites/r-typesheet30.gif", sf::Texture("content/sprites/r-typesheet30.gif"));

    sprite2D_component_s sprite_info;
    sprite_info.handle = handle;
    sprite_info.dimension = {0, 0, 162, 216}; // Approximate boss size from sprite sheet
    sprite_info.z_index = 2;
    registry.addComponent<sprite2D_component_s>(boss_id, sprite_info);

    BoxCollisionComponent collision;
    collision.tagCollision.push_back("FRIENDLY_PROJECTILE");
    collision.tagCollision.push_back("PLAYER");
    registry.addComponent<BoxCollisionComponent>(boss_id, collision);

    TagComponent tags;
    tags.tags.push_back("AI");
    tags.tags.push_back("BOSS");
    registry.addComponent<TagComponent>(boss_id, tags);
}

void EnemySpawnSystem::spawnEnemy(Registry& registry, system_context context, float x, float y, bool sine_pattern) {
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
