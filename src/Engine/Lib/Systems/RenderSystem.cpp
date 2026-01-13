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
#include "../../../RType/Common/Systems/health.hpp"

void RenderSystem::update(Registry& registry, system_context context) {
    auto& positions = registry.getView<transform_component_s>();
    auto& sprites = registry.getView<sprite2D_component_s>();
    const auto& entityIds = registry.getEntities<sprite2D_component_s>();
    for (Entity entity : entityIds) {
        if (registry.hasComponent<transform_component_s>(entity)) {
            if (!registry.hasComponent<sprite2D_component_s>(entity))
                continue;
            const transform_component_s& transform = registry.getConstComponent<transform_component_s>(entity);
            sprite2D_component_s& spriteData = registry.getComponent<sprite2D_component_s>(entity);
            drawEntity(entity, transform, spriteData, registry, context);
        }
    }

    const auto& textIds = registry.getEntities<TextComponent>();
    for (Entity entity : textIds) {
        auto& textComp = registry.getConstComponent<TextComponent>(entity);
        drawText(textComp, context);
    }

    return;
}

void RenderSystem::drawText(const TextComponent& textComp, const system_context& context) {
    if (!_fontLoaded) {
        if (!_font.openFromFile(textComp.fontPath)) {
            std::cerr << "Failed to load font: " << textComp.fontPath << std::endl;
            return;
        }
        _fontLoaded = true;
    }

    if (!_fontLoaded)
        return;

    sf::Text text(_font);
    text.setString(textComp.text);
    text.setCharacterSize(textComp.characterSize);
    text.setFillColor(textComp.color);
    text.setPosition({textComp.x, textComp.y});
    context.window.draw(text);
}

void RenderSystem::drawEntity(Entity entity, const transform_component_s& transform, sprite2D_component_s& spriteData,
                              Registry& registry, const system_context& context) {
    if (!context.texture_manager.has(spriteData.handle))
        return;

    sf::Texture& texture = context.texture_manager.get_resource(spriteData.handle).value().get();
    sf::Sprite sprite(texture);

    if (spriteData.is_animated && !spriteData.frames.empty()) {
        int frameIndex = spriteData.current_animation_frame;
        const rect& frame = spriteData.frames[frameIndex];
        sprite.setTextureRect(sf::IntRect({static_cast<int>(frame.x), static_cast<int>(frame.y)},
                                          {static_cast<int>(frame.width), static_cast<int>(frame.height)}));
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
        sprite.setTextureRect(
            sf::IntRect({static_cast<int>(spriteData.dimension.x), static_cast<int>(spriteData.dimension.y)},
                        {static_cast<int>(spriteData.dimension.width), static_cast<int>(spriteData.dimension.height)}));
    }
    sprite.setPosition({transform.x, transform.y});
    sprite.setScale({transform.scale_x, transform.scale_y});

    // Boss hit feedback (client-side): detect HP drops on entities tagged "BOSS" and draw an additive white flash.
    bool is_boss = false;
    if (registry.hasComponent<TagComponent>(entity)) {
        const auto& tags = registry.getConstComponent<TagComponent>(entity);
        for (const auto& t : tags.tags) {
            if (t == "BOSS") {
                is_boss = true;
                break;
            }
        }
    }

    bool flash_active = false;
    if (is_boss && registry.hasComponent<HealthComponent>(entity)) {
        const auto& health = registry.getConstComponent<HealthComponent>(entity);
        auto& state = _bossHitFlash[entity];

        if (state.last_hp < 0) {
            state.last_hp = health.current_hp;
        } else {
            if (health.current_hp < state.last_hp) {
                state.timer = _bossHitFlashDuration;
            }
            state.last_hp = health.current_hp;
        }

        if (state.timer > 0.0f) {
            state.timer -= context.dt;
            if (state.timer < 0.0f) {
                state.timer = 0.0f;
            }
        }

        flash_active = (state.timer > 0.0f);
    }

    context.window.draw(sprite);
    if (flash_active) {
        sf::Sprite flash = sprite;
        flash.setColor(sf::Color(255, 255, 255, 200));
        context.window.draw(flash, sf::RenderStates(sf::BlendAdd));
    }
    return;
}
