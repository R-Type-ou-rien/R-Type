/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** WallCollisionSystem implementation
*/

#include "wall_collision.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/terrain_component.hpp"
#include "health.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

void WallCollisionSystem::update(Registry& registry, system_context context) {
    auto& walls = registry.getEntities<WallComponent>();
    auto& players = registry.getEntities<TagComponent>();

    for (auto wall_entity : walls) {
        if (!registry.hasComponent<transform_component_s>(wall_entity))
            continue;
        if (!registry.hasComponent<AnimatedSprite2D>(wall_entity))
            continue;

        const auto& wall = registry.getConstComponent<WallComponent>(wall_entity);
        const auto& wall_transform = registry.getConstComponent<transform_component_s>(wall_entity);
        const auto& wall_sprite = registry.getConstComponent<AnimatedSprite2D>(wall_entity);

        float wall_width = wall.width > 0 ? wall.width : wall_sprite.animations.at(wall_sprite.currentAnimation).frames.at(wall_sprite.currentFrameIndex).width * wall_transform.scale_x;
        float wall_height = wall.height > 0 ? wall.height : wall_sprite.animations.at(wall_sprite.currentAnimation).frames.at(wall_sprite.currentFrameIndex).height * wall_transform.scale_y;

        for (auto entity : players) {
            if (entity == wall_entity)
                continue;
            if (!registry.hasComponent<transform_component_s>(entity))
                continue;
            if (!registry.hasComponent<AnimatedSprite2D>(entity))
                continue;

            const auto& tags = registry.getConstComponent<TagComponent>(entity);

            bool is_player = false;
            bool is_projectile = false;
            for (const auto& tag : tags.tags) {
                if (tag == "PLAYER") {
                    is_player = true;
                    break;
                }
                if (tag == "FRIENDLY_PROJECTILE" || tag == "ENEMY_PROJECTILE") {
                    is_projectile = true;
                    break;
                }
            }

            if (!is_player && !is_projectile)
                continue;

            auto& entity_transform = registry.getComponent<transform_component_s>(entity);
            const auto& entity_sprite = registry.getConstComponent<AnimatedSprite2D>(entity);

            float entity_width = entity_sprite.animations.at(entity_sprite.currentAnimation).frames.at(entity_sprite.currentFrameIndex).width * entity_transform.scale_x;
            float entity_height = entity_sprite.animations.at(entity_sprite.currentAnimation).frames.at(entity_sprite.currentFrameIndex).height * entity_transform.scale_y;

            float entity_left = entity_transform.x;
            float entity_right = entity_transform.x + entity_width;
            float entity_top = entity_transform.y;
            float entity_bottom = entity_transform.y + entity_height;

            float wall_left = wall_transform.x;
            float wall_right = wall_transform.x + wall_width;
            float wall_top = wall_transform.y;
            float wall_bottom = wall_transform.y + wall_height;

            bool collides = !(entity_right < wall_left || entity_left > wall_right || entity_bottom < wall_top ||
                              entity_top > wall_bottom);

            if (collides) {
                if (is_player && wall.blocks_player) {
                    std::cout << "[WallCollision] Player " << entity << " collided with wall " << wall_entity << std::endl;
                    handlePlayerWallCollision(registry, entity, wall_entity);
                } else if (is_projectile && wall.blocks_projectiles) {
                    handleProjectileWallCollision(registry, entity, wall_entity);
                }
            }
        }
    }
}

void WallCollisionSystem::handlePlayerWallCollision(Registry& registry, Entity player, Entity wall) {
    if (!registry.hasComponent<HealthComponent>(player)) {
        std::cout << "[WallCollision] Player " << player << " has no HealthComponent!" << std::endl;
        return;
    }

    auto& health = registry.getComponent<HealthComponent>(player);
    std::cout << "[WallCollision] Killing player " << player << " (HP before: " << health.current_hp << ")" << std::endl;
    health.current_hp = 0;  // Instant kill
}

void WallCollisionSystem::handleProjectileWallCollision(Registry& registry, Entity projectile, Entity wall) {
    const auto& wall_comp = registry.getConstComponent<WallComponent>(wall);

    if (!wall_comp.is_destructible) {
        registry.destroyEntity(projectile);
    }
}
