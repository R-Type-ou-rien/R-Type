/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RenderSystem.hpp
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#include "../../Components/Components.hpp"
#include "../ISystem.hpp"

class RenderSystem : public ISystem {
   public:
    explicit RenderSystem(sf::RenderWindow& window) : _window(window) {}

    void init(Registry& registry) override {}

    void update(Registry& registry, system_context context) override;

   private:
    sf::Texture& getTexture(const std::string& path);

    void drawEntity(const transform_component_s& transform, const sprite2D_component_s& spriteData,
                    const system_context& context);

   private:
    sf::RenderWindow& _window;

    std::unordered_map<std::string, sf::Texture> _textureCache;
};
