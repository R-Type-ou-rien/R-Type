/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RenderSystem.cpp
*/

#include "RenderSystem.hpp"

// Public //

void RenderSystem::update(Registry& registry, float dt) {
    _window.clear(sf::Color::Black);
    auto& positions = registry.getView<Position2D>();
    auto& sprites = registry.getView<Sprite2D>();
    const auto& entityIds = registry.getEntities<Sprite2D>();

    for (size_t i = 0; i < entityIds.size(); ++i) {
        Entity id = entityIds[i];

        if (registry.hasComponent<Position2D>(id)) {
            auto& pos = registry.getComponent<Position2D>(id);
            auto& spriteData = sprites[i];

            drawEntity(pos, spriteData);
        }
    }
    _window.display();
}

// Public //

// Private //

sf::Texture& RenderSystem::getTexture(const std::string& path) {
    if (_textureCache.find(path) == _textureCache.end()) {
        if (!_textureCache[path].loadFromFile(path)) {
            std::cerr << "Error: Could not load texture " << path << std::endl;
        }
    }
    return _textureCache[path];
}

void RenderSystem::drawEntity(const Position2D& pos, const Sprite2D& spriteData) {
    sf::Sprite sprite;
    
    sprite.setTexture(getTexture(spriteData.texturePath));

    if (spriteData.rectWidth > 0 && spriteData.rectHeight > 0) {
        sprite.setTextureRect(sf::IntRect(
            spriteData.rectLeft, spriteData.rectTop, 
            spriteData.rectWidth, spriteData.rectHeight
        ));
    }

    sprite.setPosition(pos.x, pos.y);
    sprite.setScale(spriteData.scale, spriteData.scale);

    _window.draw(sprite);
}

// Private //