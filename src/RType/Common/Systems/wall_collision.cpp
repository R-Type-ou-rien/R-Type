/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** WallCollisionSystem implementation
*/

#include "wall_collision.hpp"
#include "Components/StandardComponents.hpp"
#include "../Components/terrain_component.hpp"
#include "../../../../Engine/Lib/Components/LobbyIdComponent.hpp"
#include "../../../../Engine/Lib/Utils/LobbyUtils.hpp"
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
        if (!registry.hasComponent<sprite2D_component_s>(wall_entity))
            continue;

        uint32_t wall_lobby_id = engine::utils::getLobbyId(registry, wall_entity);

        const auto& wall = registry.getConstComponent<WallComponent>(wall_entity);
        const auto& wall_transform = registry.getConstComponent<transform_component_s>(wall_entity);
        const auto& wall_sprite = registry.getConstComponent<sprite2D_component_s>(wall_entity);

        float wall_width = wall.width > 0 ? wall.width : wall_sprite.dimension.width * wall_transform.scale_x;
        float wall_height = wall.height > 0 ? wall.height : wall_sprite.dimension.height * wall_transform.scale_y;

        for (auto entity : players) {
            uint32_t entity_lobby_id = engine::utils::getLobbyId(registry, entity);

            // Skip if different lobbies (and both are not 0)
            if (wall_lobby_id != entity_lobby_id && wall_lobby_id != 0 && entity_lobby_id != 0) {
                continue;
            }

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
    if (!registry.hasComponent<HealthComponent>(player)) {
        return;
    }

    auto& health = registry.getComponent<HealthComponent>(player);
    health.current_hp = 0;  // Instant kill
}

void WallCollisionSystem::handleProjectileWallCollision(Registry& registry, Entity projectile, Entity wall) {
    const auto& wall_comp = registry.getConstComponent<WallComponent>(wall);

    if (!wall_comp.is_destructible) {
        registry.destroyEntity(projectile);
    }
}
