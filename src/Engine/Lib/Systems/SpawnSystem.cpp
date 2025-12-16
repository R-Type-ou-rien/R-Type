/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SpawnSystem.cpp
*/

#include "SpawnSystem.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <algorithm>
#include <cstdlib>

void SpawnSystem::update(Registry& registry, system_context context) {
    const auto windowWidth = static_cast<float>(context.window.getSize().x);
    const auto windowHeight = static_cast<float>(context.window.getSize().y);

    const auto& spawners = registry.getEntities<SpawnComponent>();
    for (Entity spawner : spawners) {
        auto& spawn = registry.getComponent<SpawnComponent>(spawner);
        if (!spawn.active)
            continue;

        spawn.elapsed += context.dt;
        if (spawn.elapsed >= spawn.interval) {
            spawn.elapsed -= spawn.interval;

            Entity enemy = registry.createEntity();

            float randomY = (static_cast<float>(rand()) / RAND_MAX) * (windowHeight - spawn.frame.height * spawn.scale_y);
            float spawnX = windowWidth + 20.0f;

            transform_component_s transform{spawnX, randomY};
            transform.scale_x = spawn.scale_x;
            transform.scale_y = spawn.scale_y;
            registry.addComponent<transform_component_s>(enemy, transform);

            registry.addComponent<Velocity2D>(enemy, Velocity2D{spawn.speed_x, 0.0f});

            registry.addComponent<TagComponent>(enemy, TagComponent{{"ENEMY"}});

            sprite2D_component_s sprite{};
            sprite.animation_speed = 0.0f;
            sprite.current_animation_frame = 0;
            sprite.dimension = spawn.frame;
            if (context.texture_manager.is_loaded(spawn.sprite_path)) {
                sprite.handle = context.texture_manager.get_handle(spawn.sprite_path).value();
            } else {
                sprite.handle = context.texture_manager.load_resource(spawn.sprite_path, sf::Texture(spawn.sprite_path));
            }
            registry.addComponent<sprite2D_component_s>(enemy, sprite);
            registry.addComponent<BoxCollisionComponent>(enemy, BoxCollisionComponent{});
        }
    }

    const auto& entities = registry.getEntities<TagComponent>();
    for (Entity entity : entities) {
        if (!registry.hasComponent<transform_component_s>(entity) || !registry.hasComponent<sprite2D_component_s>(entity))
            continue;

        const auto& tags = registry.getComponent<TagComponent>(entity).tags;
        if (std::find(tags.begin(), tags.end(), "ENEMY") == tags.end())
            continue;

        const auto& transform = registry.getComponent<transform_component_s>(entity);
        const auto& sprite = registry.getComponent<sprite2D_component_s>(entity);
        float entityWidth = sprite.dimension.width * transform.scale_x;

        if (transform.x + entityWidth < 0.0f) {
            registry.destroyEntity(entity);
        }
    }
}
