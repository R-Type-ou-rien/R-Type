/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** WallCollisionSystem implementation
*/

#include "wall_collision.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/terrain_component.hpp"
#include <algorithm>
#include <cmath>

void WallCollisionSystem::update(Registry& registry, system_context context) {
    auto& walls = registry.getEntities<WallComponent>();
    auto& players = registry.getEntities<TagComponent>();

    for (auto wall_entity : walls) {
        if (!registry.hasComponent<transform_component_s>(wall_entity))
            continue;
        if (!registry.hasComponent<sprite2D_component_s>(wall_entity))
            continue;

        const auto& wall = registry.getConstComponent<WallComponent>(wall_entity);
        const auto& wall_transform = registry.getConstComponent<transform_component_s>(wall_entity);
        const auto& wall_sprite = registry.getConstComponent<sprite2D_component_s>(wall_entity);

        float wall_width = wall.width > 0 ? wall.width : wall_sprite.dimension.width * wall_transform.scale_x;
        float wall_height = wall.height > 0 ? wall.height : wall_sprite.dimension.height * wall_transform.scale_y;

        for (auto entity : players) {
            if (entity == wall_entity)
                continue;
            if (!registry.hasComponent<transform_component_s>(entity))
                continue;
            if (!registry.hasComponent<sprite2D_component_s>(entity))
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
            const auto& entity_sprite = registry.getConstComponent<sprite2D_component_s>(entity);

            float entity_width = entity_sprite.dimension.width * entity_transform.scale_x;
            float entity_height = entity_sprite.dimension.height * entity_transform.scale_y;

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
                    handlePlayerWallCollision(registry, entity, wall_entity);
                } else if (is_projectile && wall.blocks_projectiles) {
                    handleProjectileWallCollision(registry, entity, wall_entity);
                }
            }
        }
    }
}

void WallCollisionSystem::handlePlayerWallCollision(Registry& registry, Entity player, Entity wall) {
    if (!registry.hasComponent<transform_component_s>(player))
        return;
    if (!registry.hasComponent<transform_component_s>(wall))
        return;
    if (!registry.hasComponent<sprite2D_component_s>(player))
        return;
    if (!registry.hasComponent<sprite2D_component_s>(wall))
        return;

    auto& player_transform = registry.getComponent<transform_component_s>(player);
    const auto& wall_transform = registry.getConstComponent<transform_component_s>(wall);
    const auto& wall_comp = registry.getConstComponent<WallComponent>(wall);
    const auto& player_sprite = registry.getConstComponent<sprite2D_component_s>(player);
    const auto& wall_sprite = registry.getConstComponent<sprite2D_component_s>(wall);

    float player_width = player_sprite.dimension.width * player_transform.scale_x;
    float player_height = player_sprite.dimension.height * player_transform.scale_y;
    float wall_width = wall_comp.width > 0 ? wall_comp.width : wall_sprite.dimension.width * wall_transform.scale_x;
    float wall_height = wall_comp.height > 0 ? wall_comp.height : wall_sprite.dimension.height * wall_transform.scale_y;

    float player_center_x = player_transform.x + player_width / 2;
    float player_center_y = player_transform.y + player_height / 2;
    float wall_center_x = wall_transform.x + wall_width / 2;
    float wall_center_y = wall_transform.y + wall_height / 2;

    float overlap_x = (player_width / 2 + wall_width / 2) - std::abs(player_center_x - wall_center_x);
    float overlap_y = (player_height / 2 + wall_height / 2) - std::abs(player_center_y - wall_center_y);

    if (overlap_x < overlap_y) {
        if (player_center_x < wall_center_x) {
            player_transform.x -= overlap_x;
        } else {
            player_transform.x += overlap_x;
        }
    } else {
        if (player_center_y < wall_center_y) {
            player_transform.y -= overlap_y;
        } else {
            player_transform.y += overlap_y;
        }
    }

    if (registry.hasComponent<Velocity2D>(player)) {
        auto& velocity = registry.getComponent<Velocity2D>(player);
        if (overlap_x < overlap_y) {
            velocity.vx = 0;
        } else {
            velocity.vy = 0;
        }
    }
}

void WallCollisionSystem::handleProjectileWallCollision(Registry& registry, Entity projectile, Entity wall) {
    const auto& wall_comp = registry.getConstComponent<WallComponent>(wall);

    if (!wall_comp.is_destructible) {
        registry.destroyEntity(projectile);
    }
}
