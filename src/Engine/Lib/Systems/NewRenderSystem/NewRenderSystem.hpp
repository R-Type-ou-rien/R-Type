/*
** EPITECH PROJECT, 2025
** Smash
** File description:
** RenderSystem.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

#include "../../Components/Sprite/Sprite2D.hpp"
#include "../../Components/Sprite/AnimatedSprite2D.hpp"

#include "Components/StandardComponents.hpp"
#include "ISystem.hpp"

class NewRenderSystem : public ISystem {
   public:
    NewRenderSystem() = default;

    void update(Registry& registry, system_context context) override;

   private:
    struct DrawCmd {
        int layer;
        std::uint64_t order;
        sf::Sprite sprite;
    };

    void drawSpriteEntity(const TransformComponent& transform, const Sprite2D& spriteData,
                          const system_context& context, std::vector<DrawCmd>& out, std::uint64_t& order);

    void drawAnimatedSpriteEntity(const TransformComponent& transform, const AnimatedSprite2D& spriteData,
                                  const system_context& context, std::vector<DrawCmd>& out, std::uint64_t& order);

    void drawText(const TextComponent& textComp, const system_context& context);

   private:
    sf::Font _font;
    bool _fontLoaded = false;
};
