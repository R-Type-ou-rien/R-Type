/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RenderSystem.cpp
*/

#include "RenderSystem.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <iostream>
#include <iterator>

// Public //

void RenderSystem::update(Registry& registry, system_context context) {
    auto& positions = registry.getView<transform_component_s>();
    auto& sprites = registry.getView<sprite2D_component_s>();
    const auto& entityIds = registry.getEntities<sprite2D_component_s>();
    for (Entity entity : entityIds) {
        if (registry.hasComponent<transform_component_s>(entity)) {
            if (!registry.hasComponent<sprite2D_component_s>(entity))
                continue;
            transform_component_s& transform = registry.getComponent<transform_component_s>(entity);
            sprite2D_component_s& spriteData = registry.getComponent<sprite2D_component_s>(entity);
            drawEntity(transform, spriteData, context);
        }
    }
    return;
}

void RenderSystem::drawEntity(const transform_component_s& transform, const sprite2D_component_s& spriteData,
                              const system_context& context) {
    if (!context.texture_manager.has_resource(spriteData.handle))
        return;

    sf::Texture texture = context.texture_manager.get_resource(spriteData.handle).value();
    sf::Sprite sprite(texture);

    if (spriteData.dimension.size.x > 0 && spriteData.dimension.size.y > 0)
        sprite.setTextureRect(spriteData.dimension);

    sprite.setPosition({transform.x, transform.y});
    sprite.setScale(transform.scale);

    context.window.draw(sprite);
    return;
}

// Private //