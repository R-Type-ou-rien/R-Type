/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** NewRenderSystem.cpp
*/

#include "NewRenderSystem.hpp"
#include "Context.hpp"

#include <algorithm>
#include <iostream>

static void applyFlip(sf::Sprite& spr, bool flipX, bool flipY) {
    const auto rect = spr.getTextureRect();

    spr.setOrigin({static_cast<float>(rect.size.x) * 0.5f, static_cast<float>(rect.size.y) * 0.5f});

    spr.setScale({(flipX ? -1.f : 1.f), (flipY ? -1.f : 1.f)});
}

void NewRenderSystem::update(Registry& registry, system_context context) {
    std::vector<DrawCmd> cmds;
    const auto& spriteEntities = registry.getEntities<Sprite2D>();
    const auto& animEntities = registry.getEntities<AnimatedSprite2D>();
    const auto& textIds = registry.getEntities<TextComponent>();
    cmds.reserve(spriteEntities.size() + animEntities.size());
    std::uint64_t order = 0;

    for (Entity e : spriteEntities) {
        if (!registry.hasComponent<transform_component_s>(e))
            continue;

        const auto& tr = registry.getComponent<transform_component_s>(e);
        const auto& sp = registry.getComponent<Sprite2D>(e);
        drawSpriteEntity(tr, sp, context, cmds, order);
    }

    for (Entity e : animEntities) {
        if (!registry.hasComponent<transform_component_s>(e))
            continue;

        const auto& tr = registry.getComponent<transform_component_s>(e);
        const auto& sp = registry.getComponent<AnimatedSprite2D>(e);
        drawAnimatedSpriteEntity(tr, sp, context, cmds, order);
    }

    std::stable_sort(cmds.begin(), cmds.end(), [](const DrawCmd& a, const DrawCmd& b) {
        if (a.layer != b.layer)
            return a.layer < b.layer;
        return a.order < b.order;
    });

    for (auto& cmd : cmds)
        context.window.draw(cmd.sprite);

    for (Entity entity : textIds) {
        auto& textComp = registry.getComponent<TextComponent>(entity);
        drawText(textComp, context);
    }
}

void NewRenderSystem::drawSpriteEntity(const transform_component_s& transform, const Sprite2D& spriteData,
                                       const system_context& context, std::vector<DrawCmd>& out, std::uint64_t& order) {
    if (!context.texture_manager.has(spriteData.handle))
        return;

    sf::Texture& texture = context.texture_manager.get_resource(spriteData.handle).value().get();
    sf::Sprite spr(texture);

    spr.setTextureRect(
        sf::IntRect({spriteData.rect.x, spriteData.rect.y}, {spriteData.rect.width, spriteData.rect.height}));

    applyFlip(spr, spriteData.flipX, spriteData.flipY);

    spr.setPosition({transform.x, transform.y});
    spr.setScale({transform.scale_x * spr.getScale().x, transform.scale_y * spr.getScale().y});

    out.push_back(DrawCmd{static_cast<int>(spriteData.layer), order++, std::move(spr)});
}

void NewRenderSystem::drawAnimatedSpriteEntity(const transform_component_s& transform,
                                               const AnimatedSprite2D& spriteData, const system_context& context,
                                               std::vector<DrawCmd>& out, std::uint64_t& order) {
    if (spriteData.currentAnimation.empty() ||
        spriteData.animations.find(spriteData.currentAnimation) == spriteData.animations.end())
        return;

    const auto& clip = spriteData.animations.at(spriteData.currentAnimation);

    if (!context.texture_manager.has(clip.handle))
        return;

    if (spriteData.currentFrameIndex >= clip.frames.size())
        return;

    sf::Texture& texture = context.texture_manager.get_resource(clip.handle).value().get();
    sf::Sprite spr(texture);

    spr.setTextureRect(sf::IntRect(
        {clip.frames[spriteData.currentFrameIndex].x, clip.frames[spriteData.currentFrameIndex].y},
        {clip.frames[spriteData.currentFrameIndex].width, clip.frames[spriteData.currentFrameIndex].height}));

    applyFlip(spr, spriteData.flipX, spriteData.flipY);

    spr.setPosition({transform.x, transform.y});
    spr.setScale({transform.scale_x * spr.getScale().x, transform.scale_y * spr.getScale().y});

    out.push_back(DrawCmd{static_cast<int>(spriteData.layer), order++, std::move(spr)});
}

void NewRenderSystem::drawText(const TextComponent& textComp, const system_context& context) {
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
