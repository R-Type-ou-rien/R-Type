#include "BackgroundSystem.hpp"
#include <SFML/Graphics.hpp>
#include "Components/StandardComponents.hpp"
#include <iostream>

void BackgroundSystem::update(Registry& registry, system_context context) {
    const auto& entities = registry.getEntities<BackgroundComponent>();
    if (entities.empty())
        return;

    const float windowHeight = static_cast<float>(context.window.getSize().y);
    const float windowWidth = static_cast<float>(context.window.getSize().x);

    for (Entity entity : entities) {
        auto& bg = registry.getComponent<BackgroundComponent>(entity);
        bg.x_offset -= bg.scroll_speed * context.dt; // move left
        if (!context.texture_manager.has_resource(bg.texture_handle))
            continue;
        sf::Texture& texture = context.texture_manager.get_resource(bg.texture_handle).value().get();
        texture.setRepeated(true);
        sf::Sprite sprite(texture);
        float scale = windowHeight / sprite.getLocalBounds().size.y;
        sprite.setScale({scale, scale});
        float scaledWidth = sprite.getGlobalBounds().size.x;
        if (bg.x_offset <= -scaledWidth)
            bg.x_offset += scaledWidth;
        const int tilesToDraw = static_cast<int>(windowWidth / scaledWidth) + 3;
        float startX = bg.x_offset - scaledWidth;
        for (int i = 0; i < tilesToDraw; ++i) {
            sprite.setPosition({startX + i * scaledWidth, 0.f});
            context.window.draw(sprite);
        }
    }
}
