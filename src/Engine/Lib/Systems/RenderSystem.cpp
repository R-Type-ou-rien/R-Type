 

#include "RenderSystem.hpp"
#include "registry.hpp"
#include "Components/StandardComponents.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <iostream>
#include <iterator>
#include <ostream>

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

    const auto& textIds = registry.getEntities<TextComponent>();
    for (Entity entity : textIds) {
        auto& textComp = registry.getComponent<TextComponent>(entity);
        drawText(textComp, context);
    }

    return;
}

void RenderSystem::drawText(const TextComponent& textComp, const system_context& context) {
    if (!context.window.has_value())
        return;

    sf::Font font;
    if (!font.openFromFile(textComp.fontPath)) {
        std::cerr << "Failed to load font: " << textComp.fontPath << std::endl;
        return;
    }

    sf::Text text(font);
    text.setString(textComp.text);
    text.setCharacterSize(textComp.characterSize);
    text.setFillColor(textComp.color);
    text.setPosition({textComp.x, textComp.y});

    context.window.value().get().draw(text);
}

void RenderSystem::drawEntity(const transform_component_s& transform, sprite2D_component_s& spriteData,
                              const system_context& context) {
    if (!context.window.has_value()) {
        return;
    }
    sf::RenderWindow& window = context.window.value();

    if (!context.texture_manager.has_resource(spriteData.handle))
        return;

    sf::Texture& texture = context.texture_manager.get_resource(spriteData.handle).value().get();
    sf::Sprite sprite(texture);

    if (spriteData.is_animated && !spriteData.frames.empty()) {
        int frameIndex = spriteData.current_animation_frame;
        const rect& frame = spriteData.frames[frameIndex];
        sprite.setTextureRect(sf::IntRect({int(frame.x), int(frame.y)}, {int(frame.width), int(frame.height)}));
        spriteData.lastUpdateTime += context.dt;
        if (spriteData.lastUpdateTime >= spriteData.animation_speed) {
            if (spriteData.reverse_animation && spriteData.current_animation_frame <= 0) {
                spriteData.current_animation_frame += 1;
                spriteData.reverse_animation = false;
            } else if (spriteData.current_animation_frame >= spriteData.frames.size() - 1) {
                if (spriteData.loop_animation) {
                    spriteData.current_animation_frame = 0;
                } else {
                    spriteData.reverse_animation = true;
                    spriteData.current_animation_frame -= 1;
                }
            } else if (spriteData.reverse_animation) {
                spriteData.current_animation_frame -= 1;
            } else {
                spriteData.current_animation_frame += 1;
            }
            spriteData.lastUpdateTime = 0.f;
        }
    } else if (spriteData.dimension.width > 0 && spriteData.dimension.height > 0) {
        sprite.setTextureRect(sf::IntRect({int(spriteData.dimension.x), int(spriteData.dimension.y)},
                                          {int(spriteData.dimension.width), int(spriteData.dimension.height)}));
    }
    sprite.setPosition({transform.x, transform.y});
    sprite.setScale({transform.scale_x, transform.scale_y});
    window.draw(sprite);
    return;
}
